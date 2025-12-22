#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdarg>
#include <exception>
#include <iostream>
#include <sstream>
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
    instr_info info;
    char *buf = nullptr;
    size_t len = 0;
    FILE *mem = open_memstream(&buf, &len);
    if (mem == nullptr) {
        throw std::runtime_error("open_memstream failed");
    }
    if (decode_instruction(bf, offset, mem, &info) != 0) {
        fclose(mem);
        free(buf);
        throw std::runtime_error("failed to decode instruction");
    }
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

struct Cache {
    std::unordered_map<uint32_t, std::string> disasm;
};

const std::string &disasm_text(bytefile *bf, uint32_t offset, Cache &cache) {
    auto it = cache.disasm.find(offset);
    if (it != cache.disasm.end()) {
        return it->second;
    }
    std::string text = strip_address(disasm_once(bf, offset));
    auto res = cache.disasm.emplace(offset, std::move(text));
    return res.first->second;
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

        Cache cache;
        std::unordered_map<std::string, uint64_t> freq;

        auto add_pair = [&](const std::string &a, const std::string &b) {
            std::string key = a + " -> " + b;
            freq[key] += 1;
        };
        auto add_uni = [&](const std::string &a) { freq[a] += 1; };

        std::string last;
        bool has_last = false;

        for (uint32_t off = 0; off < code_size;) {
            instr_info info;
            if (decode_instruction(bf, off, nullptr, &info) != 0) {
                throw std::runtime_error("failed to decode instruction");
            }
            if (info.size == 0 || info.next_offset > code_size) {
                throw std::runtime_error("invalid instruction size");
            }

            const std::string &curr = disasm_text(bf, off, cache);
            add_uni(curr);

            if (has_last) {
                add_pair(last, curr);
            }

            for (uint8_t i = 0; i < info.target_count; ++i) {
                uint32_t tgt = info.targets[i];
                const std::string &tgt_text = disasm_text(bf, tgt, cache);
                add_pair(curr, tgt_text);
            }

            OpGroup g = static_cast<OpGroup>(info.group);
            uint8_t l = info.opcode;
            bool reset = false;
            bool pseudo_end = false;

            if (g == OpGroup::Storage && l == static_cast<uint8_t>(StorageOpcode::Jump)) {
                reset = true;
            } else if (g == OpGroup::Control && l == static_cast<uint8_t>(ControlOpcode::Call)) {
                pseudo_end = true;
            } else if (g == OpGroup::Control &&
                       l == static_cast<uint8_t>(ControlOpcode::CallClosure)) {
                pseudo_end = true;
            } else if (info.breaks_flow) {
                reset = true;
            }

            if (pseudo_end) {
                last = "END";
                has_last = true;
            } else if (reset) {
                has_last = false;
            } else {
                last = curr;
                has_last = true;
            }

            off = info.next_offset;
        }

        std::vector<std::pair<std::string, uint64_t>> ordered(freq.begin(), freq.end());
        std::sort(ordered.begin(), ordered.end(), [](const auto &lhs, const auto &rhs) {
            if (lhs.second != rhs.second) {
                return lhs.second > rhs.second;
            }
            return lhs.first < rhs.first;
        });

        for (const auto &entry : ordered) {
            std::cout << entry.first << " : " << entry.second << "\n";
        }
    } catch (const std::exception &e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
