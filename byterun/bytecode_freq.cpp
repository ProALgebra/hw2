#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <exception>
#include <iostream>
#include <queue>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

extern "C" void failure(const char *fmt, ...);

#include "byterun.h"

extern "C" void failure(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), fmt, ap);
    va_end(ap);
    throw std::runtime_error(buffer);
}

namespace {

enum class OpGroup : uint8_t {
    BinOp = 0,
    Storage = 1,
    Load = 2,
    LoadAddress = 3,
    Store = 4,
    Control = 5,
    Pattern = 6,
    Builtin = 7,
    Halt = 15
};

enum class StorageOpcode : uint8_t { Jump = 5 };
enum class ControlOpcode : uint8_t {
    JumpIfZero = 0,
    JumpIfNotZero = 1,
    CallClosure = 5,
    Call = 6
};

constexpr size_t MAX_INS_BYTES = 64;

void disasm_once(bytefile *bf, uint32_t offset, FILE *out) {
    instr_info info;
    if (decode_instruction(bf, offset, out, &info) != 0) {
        throw std::runtime_error("failed to decode instruction");
    }
}

const char *disasm_to_string(bytefile *bf, uint32_t offset) {
    static thread_local char buf[256];
    std::fill(std::begin(buf), std::end(buf), 0);
    if (decode_instruction_char(bf, offset, buf, sizeof buf, nullptr) != 0) {
        throw std::runtime_error("failed to decode instruction");
    }
    return buf;
}

std::string strip_address(const char *s) {
    std::string out;
    bool seen_colon = false;
    bool skipping = true;
    for (const char *p = s; *p; ++p) {
        char c = *p;
        if (!seen_colon) {
            if (c == ':') {
                seen_colon = true;
                skipping = true;
            }
            continue;
        }
        if (skipping) {
            if (c == ' ' || c == '\t') {
                continue;
            }
            skipping = false;
        }
        if (c == '\n' || c == '\r') {
            break;
        }
        out.push_back(c);
    }
    if (out.empty()) {
        return std::string(s);
    }
    return out;
}

struct Key {
    uint8_t first[MAX_INS_BYTES];
    uint8_t second[MAX_INS_BYTES];
    uint8_t len1 = 0;
    uint8_t len2 = 0;
    bool has_second = false;
};

struct KeyHash {
    std::size_t operator()(const Key &k) const noexcept {
        auto hash_bytes = [](const uint8_t *data, uint8_t len) -> std::size_t {
            std::size_t h = 0;
            for (uint8_t i = 0; i < len; ++i) {
                h = (h * 131) ^ data[i];
            }
            return h;
        };
        std::size_t h1 = hash_bytes(k.first, k.len1);
        std::size_t h2 = k.has_second ? hash_bytes(k.second, k.len2) : 0;
        return h1 ^ (h2 << 1) ^ static_cast<std::size_t>(k.has_second);
    }
};

struct KeyEq {
    bool operator()(const Key &a, const Key &b) const noexcept {
        if (a.has_second != b.has_second || a.len1 != b.len1 || a.len2 != b.len2) {
            return false;
        }
        if (!std::equal(a.first, a.first + a.len1, b.first)) {
            return false;
        }
        if (a.has_second) {
            if (!std::equal(a.second, a.second + a.len2, b.second)) {
                return false;
            }
        }
        return true;
    }
};

struct Example {
    uint32_t off1;
    uint32_t off2;
    bool has_second;
};

Key make_key_bytes(const char *ptr, uint32_t len) {
    if (len > MAX_INS_BYTES) {
        throw std::runtime_error("instruction too long for fixed buffer");
    }
    Key k;
    k.len1 = static_cast<uint8_t>(len);
    for (uint32_t i = 0; i < len; ++i) {
        k.first[i] = static_cast<uint8_t>(ptr[i]);
    }
    k.has_second = false;
    k.len2 = 0;
    return k;
}

} // namespace

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <bytecode.bc>\n";
        return 1;
    }

    try {
        bytefile *bf = read_file(argv[1]);
        uint32_t code_size = bf->code_size;

        std::unordered_map<Key, uint64_t, KeyHash, KeyEq> freq;
        std::unordered_map<Key, Example, KeyHash, KeyEq> examples;
        std::unordered_map<uint32_t, instr_info> decoded;
        std::unordered_map<uint32_t, Key> bytes_cache;
        std::vector<uint8_t> seen(code_size, 0);
        std::queue<uint32_t> q;

        auto enqueue = [&](uint32_t off) {
            if (off < code_size && !seen[off]) {
                q.push(off);
            }
        };

        if (bf->public_symbols_number == 0) {
            enqueue(0);
        } else {
            for (uint32_t i = 0; i < bf->public_symbols_number; ++i) {
                enqueue(get_public_offset(bf, i));
            }
        }

        while (!q.empty()) {
            uint32_t off = q.front();
            q.pop();
            if (off >= code_size || seen[off]) {
                continue;
            }
            instr_info info;
            if (decode_instruction(bf, off, nullptr, &info) != 0) {
                throw std::runtime_error("failed to decode instruction");
            }
            if (info.size == 0 || info.next_offset > code_size) {
                throw std::runtime_error("invalid instruction size");
            }
            seen[off] = 1;
            decoded.emplace(off, info);
            bytes_cache.emplace(off, make_key_bytes(bf->code_ptr + off, info.size));

            OpGroup g = static_cast<OpGroup>(info.group);
            uint8_t l = info.opcode;
            bool allow_fallthrough = !info.breaks_flow;
            if (g == OpGroup::Control &&
                (l == static_cast<uint8_t>(ControlOpcode::Call) ||
                 l == static_cast<uint8_t>(ControlOpcode::CallClosure))) {
                allow_fallthrough = true;
            }

            if (allow_fallthrough && info.next_offset < code_size) {
                enqueue(info.next_offset);
            }
            for (uint8_t i = 0; i < info.target_count; ++i) {
                enqueue(info.targets[i]);
            }
        }

        auto make_key = [&](const Key &a, const Key &b, bool has_second) {
            Key k = a;
            k.has_second = has_second;
            if (has_second) {
                k.len2 = b.len1;
                if (k.len2 > MAX_INS_BYTES) {
                    throw std::runtime_error("instruction too long for pair buffer");
                }
                std::memcpy(k.second, b.first, k.len2);
            } else {
                k.len2 = 0;
            }
            return k;
        };

        std::vector<uint32_t> order;
        order.reserve(decoded.size());
        for (const auto &kv : decoded) {
            order.push_back(kv.first);
        }
        std::sort(order.begin(), order.end());

        Key last_key{};
        bool has_last = false;
        Example last_example{0, 0, false};

        auto record = [&](const Key &k, const Example &ex) {
            freq[k] += 1;
            if (examples.find(k) == examples.end()) {
                examples.emplace(k, ex);
            }
        };

        for (uint32_t off : order) {
            const instr_info &info = decoded.at(off);
            const Key &curr_bytes = bytes_cache.at(off);

            Key uni = make_key(curr_bytes, Key{}, false);
            record(uni, Example{off, 0, false});

            if (has_last) {
                Key pair = make_key(last_key, curr_bytes, true);
                record(pair, Example{last_example.off1, off, true});
            }

            for (uint8_t i = 0; i < info.target_count; ++i) {
                uint32_t tgt = info.targets[i];
                if (tgt < code_size && seen[tgt]) {
                    const Key &tgt_bytes = bytes_cache.at(tgt);
                    Key pair = make_key(curr_bytes, tgt_bytes, true);
                    record(pair, Example{off, tgt, true});
                }
            }

            OpGroup g = static_cast<OpGroup>(info.group);
            uint8_t l = info.opcode;
            bool reset = info.breaks_flow;
            bool pseudo_end = false;
            if (g == OpGroup::Storage && l == static_cast<uint8_t>(StorageOpcode::Jump)) {
                reset = true;
            } else if (g == OpGroup::Control &&
                       (l == static_cast<uint8_t>(ControlOpcode::Call) ||
                        l == static_cast<uint8_t>(ControlOpcode::CallClosure))) {
                reset = false;
                pseudo_end = true;
            }

            if (pseudo_end) {
                Key end_key = make_key_bytes("END", 3);
                last_key = end_key;
                last_example = Example{UINT32_MAX, 0, false};
                has_last = true;
            } else if (reset) {
                has_last = false;
            } else {
                last_key = uni;
                last_example = Example{off, 0, false};
                has_last = true;
            }
        }

        std::vector<std::pair<Key, uint64_t>> ordered(freq.begin(), freq.end());
        std::sort(ordered.begin(), ordered.end(), [](const auto &lhs, const auto &rhs) {
            if (lhs.second != rhs.second) {
                return lhs.second > rhs.second;
            }
            if (lhs.first.has_second != rhs.first.has_second) {
                return lhs.first.has_second < rhs.first.has_second;
            }
            if (lhs.first.len1 != rhs.first.len1) {
                return lhs.first.len1 < rhs.first.len1;
            }
            int cmp_first = std::memcmp(lhs.first.first, rhs.first.first, lhs.first.len1);
            if (cmp_first != 0) {
                return cmp_first < 0;
            }
            if (lhs.first.has_second) {
                if (lhs.first.len2 != rhs.first.len2) {
                    return lhs.first.len2 < rhs.first.len2;
                }
                int cmp_second = std::memcmp(lhs.first.second, rhs.first.second, lhs.first.len2);
                if (cmp_second != 0) {
                    return cmp_second < 0;
                }
            }
            return false;
        });

        for (const auto &entry : ordered) {
            const Key &k = entry.first;
            auto ex_it = examples.find(k);
            std::string lhs_text = "UNKNOWN";
            std::string rhs_text;
            if (ex_it != examples.end()) {
                const Example &ex = ex_it->second;
                lhs_text = (ex.off1 == UINT32_MAX) ? "END" : strip_address(disasm_to_string(bf, ex.off1));
                if (ex.has_second) {
                    rhs_text = strip_address(disasm_to_string(bf, ex.off2));
                }
            }
            if (k.has_second) {
                std::cout << lhs_text << " -> " << rhs_text << " : " << entry.second << "\n";
            } else {
                std::cout << lhs_text << " : " << entry.second << "\n";
            }
        }
    } catch (const std::exception &e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
