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

std::string sanitize_disasm(const std::string &s) {
    std::string out;
    out.reserve(s.size());
    bool in_space = false;
    for (char c : s) {
        if (c == '\t' || c == ' ' || c == '\n' || c == '\r') {
            if (!in_space) {
                out.push_back(' ');
                in_space = true;
            }
        } else {
            out.push_back(c);
            in_space = false;
        }
    }
    while (!out.empty() && out.back() == ' ') {
        out.pop_back();
    }
    return out;
}

std::string disasm_once(bytefile *bf, uint32_t offset) {
    char *buf = nullptr;
    size_t len = 0;
    FILE *mem = open_memstream(&buf, &len);
    if (mem == nullptr) {
        throw std::runtime_error("open_memstream failed");
    }
    disasm_single_instruction(bf, offset, mem);
    fflush(mem);
    std::string result(buf, len);
    fclose(mem);
    free(buf);
    return result;
}

std::string strip_address(const std::string &s) {
    std::string cleaned = sanitize_disasm(s);
    auto pos = cleaned.find(':');
    if (pos == std::string::npos) {
        return cleaned;
    }
    size_t start = pos + 1;
    while (start < cleaned.size() && cleaned[start] == ' ') {
        ++start;
    }
    return cleaned.substr(start);
}

struct Key {
    uint32_t off1 = 0;
    uint32_t off2 = 0;
    bool has_second = false;
};

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <bytecode.bc>\n";
        return 1;
    }

    try {
        bytefile *bf = read_file(argv[1]);
        uint32_t code_size = bf->code_size;

        std::vector<bool> reachable(code_size, 0); 
        std::vector<bool> label(code_size, 0);      
        std::queue<uint32_t> q;

        auto enqueue = [&](uint32_t off) {
            if (off < code_size && !reachable[off]) {
                q.push(off);
            }
        };

        if (bf->public_symbols_number == 0) {
            enqueue(0);
            label[0] = 1;
        } else {
            for (uint32_t i = 0; i < bf->public_symbols_number; ++i) {
                uint32_t off = get_public_offset(bf, i);
                enqueue(off);
                if (off < code_size) {
                    label[off] = 1;
                }
            }
        }

        while (!q.empty()) {
            uint32_t off = q.front();
            q.pop();
            if (off >= code_size || reachable[off]) {
                continue;
            }
            instr_info info;
            if (decode_instruction_info(bf, off, &info) != 0) {
                throw std::runtime_error("failed to decode instruction");
            }
            if (info.size == 0 || info.next_offset > code_size) {
                throw std::runtime_error("invalid instruction size");
            }
            reachable[off] = 1;

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
                uint32_t tgt = info.targets[i];
                if (tgt < code_size) {
                    label[tgt] = 1;
                }
                enqueue(tgt);
            }
            if (g == OpGroup::Control &&
                (l == static_cast<uint8_t>(ControlOpcode::Call) ||
                 l == static_cast<uint8_t>(ControlOpcode::CallClosure))) {
                if (info.next_offset < code_size) {
                    label[info.next_offset] = 1;
                }
            }
        }
        const uint8_t *code_base = reinterpret_cast<const uint8_t *>(bf->code_ptr);

        auto get_len = [&](uint32_t off) -> uint8_t {
            instr_info inf;
            if (decode_instruction_info(bf, off, &inf) != 0) {
                throw std::runtime_error("failed to decode instruction length");
            }
            return static_cast<uint8_t>(inf.size);
        };

        // Сборка ключа для биграммы
        auto make_pair_key = [&](const Key &a, const Key &b) -> Key {
            Key k = a;
            k.has_second = true;
            k.off2 = b.off1;
            return k;
        };

        struct EntryOut { Key key; uint64_t cnt; };
        std::vector<EntryOut> entries;

        auto cmp_key = [&](const Key &a, const Key &b) {
            if (a.has_second != b.has_second) return a.has_second < b.has_second;
            uint8_t len1a = get_len(a.off1);
            uint8_t len1b = get_len(b.off1);
            if (len1a != len1b) return len1a < len1b;
            int cmp_first = std::memcmp(code_base + a.off1, code_base + b.off1, len1a);
            if (cmp_first != 0) return cmp_first < 0;
            if (a.has_second) {
                uint8_t len2a = get_len(a.off2);
                uint8_t len2b = get_len(b.off2);
                if (len2a != len2b) return len2a < len2b;
                int cmp_second = std::memcmp(code_base + a.off2, code_base + b.off2, len2a);
                if (cmp_second != 0) return cmp_second < 0;
            }
            return false;
        };

        auto eq_key = [&](const Key &a, const Key &b) -> bool {
            return !cmp_key(a, b) && !cmp_key(b, a);
        };

        auto add_or_inc = [&](const Key &k) {
            auto it = std::lower_bound(entries.begin(), entries.end(), k,
                                       [&](const EntryOut &e, const Key &kk) { return cmp_key(e.key, kk); });
            if (it != entries.end() && eq_key(it->key, k)) {
                it->cnt += 1;
            } else {
                entries.insert(it, EntryOut{k, 1});
            }
        };

        uint32_t last_off = UINT32_MAX;
        instr_info last_info{};
        Key last_key{};
        uint32_t off = 0;
        while (off < code_size) {
            if (!reachable[off]) {
                ++off;
                continue;
            }
            instr_info info;
            if (decode_instruction_info(bf, off, &info) != 0) {
                throw std::runtime_error("failed to decode instruction");
            }
            Key kcur;
            kcur.off1 = off;
            kcur.has_second = false;
            kcur.off2 = 0;

            add_or_inc(kcur);

            bool is_lbl = label[off] != 0;
            if (last_off != UINT32_MAX && !is_lbl && last_info.next_offset == off) {
                Key pair = make_pair_key(last_key, kcur);
                add_or_inc(pair);
            }

            for (uint8_t t = 0; t < info.target_count; ++t) {
                uint32_t tgt = info.targets[t];
                if (tgt < code_size && reachable[tgt]) {
                    instr_info tgt_info;
                    if (decode_instruction_info(bf, tgt, &tgt_info) != 0) {
                        throw std::runtime_error("failed to decode instruction");
                    }
                    Key ktgt;
                    ktgt.off1 = tgt;
                    ktgt.has_second = false;
                    ktgt.off2 = 0;
                    Key pair = make_pair_key(kcur, ktgt);
                    add_or_inc(pair);
                }
            }

            if (info.next_offset < code_size && reachable[info.next_offset] &&
                !label[info.next_offset]) {
                last_off = off;
                last_info = info;
                last_key = kcur;
            } else {
                last_off = UINT32_MAX;
            }

            ++off;
        }
        std::sort(entries.begin(), entries.end(), [&](const EntryOut &a, const EntryOut &b) {
            if (a.cnt != b.cnt) return a.cnt > b.cnt;
            return cmp_key(a.key, b.key);
        });
        for (const auto &e : entries) {
            if (!e.key.has_second) {
                std::cout << strip_address(disasm_once(bf, e.key.off1)) << " : " << e.cnt << "\n";
            } else {
                std::cout << strip_address(disasm_once(bf, e.key.off1)) << " -> "
                          << strip_address(disasm_once(bf, e.key.off2)) << " : " << e.cnt << "\n";
            }
        }
    } catch (const std::exception &e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
