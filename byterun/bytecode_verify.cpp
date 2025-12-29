#include <cstdint>
#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <exception>
#include <fstream>
#include <iostream>
#include <queue>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

extern "C" void failure(const char *fmt, ...);

#include "byterun.h"
#include "opcodes.h"

namespace {
void fail(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), fmt, ap);
    va_end(ap);
    throw std::runtime_error(buffer);
}
} // namespace

namespace {

constexpr uint32_t kMaxDepth = 65535;
constexpr uint32_t kScopeWordCount = 10; // from interpretatorv1_0.cpp

struct Instr {
    uint32_t offset = 0;
    uint32_t size = 0;
    uint32_t next_offset = 0;
    uint32_t targets[2] = {0, 0};
    uint8_t target_count = 0;
    bool breaks_flow = false;
    OpcodeGroup group = OpcodeGroup::Halt;
    uint8_t opcode = 0;
    int32_t imm0 = 0;
    int32_t imm1 = 0;
    uint32_t imm0_offset = 0;
    uint8_t imm_count = 0;
};

struct Frame {
    uint32_t begin_offset = 0;
    uint32_t entry_depth = 0;
    uint32_t arg_slots = 0;
    uint32_t var_slots = 0;
    uint32_t captured = 0;
    uint32_t max_depth = 0;
    bool is_captured = false;
};

struct State {
    uint32_t offset = 0;
    uint32_t depth = 0;
    std::vector<Frame> frames;
};

bool same_frame_stack(const std::vector<Frame> &a, const std::vector<Frame> &b) {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i) {
        if (a[i].begin_offset != b[i].begin_offset || a[i].arg_slots != b[i].arg_slots ||
            a[i].var_slots != b[i].var_slots || a[i].captured != b[i].captured ||
            a[i].is_captured != b[i].is_captured || a[i].entry_depth != b[i].entry_depth) {
            return false;
        }
    }
    return true;
}

struct StateKeyHash {
    size_t operator()(State const &s) const noexcept {
        size_t h = s.offset;
        for (auto const &f : s.frames) {
            h ^= (static_cast<size_t>(f.begin_offset) + 0x9e3779b9 + (h << 6) + (h >> 2));
            h ^= (static_cast<size_t>(f.entry_depth) + 0x9e3779b9 + (h << 6) + (h >> 2));
        }
        return h;
    }
};

struct StateKeyEq {
    bool operator()(State const &a, State const &b) const noexcept {
        return a.offset == b.offset && same_frame_stack(a.frames, b.frames);
    }
};

Instr decode_instr(bytefile *bf, uint32_t offset, std::unordered_map<uint32_t, uint32_t> &closure_captures) {
    instr_info info;
    if (decode_instruction_info(bf, offset, &info) != 0) {
        fail("failed to decode instruction at 0x%x\n", offset);
    }
    Instr res;
    res.offset = offset;
    res.size = info.size;
    res.next_offset = info.next_offset;
    res.target_count = info.target_count;
    res.breaks_flow = info.breaks_flow != 0;
    res.group = static_cast<OpcodeGroup>(info.group);
    res.opcode = info.opcode;
    for (uint8_t i = 0; i < info.target_count; ++i) res.targets[i] = info.targets[i];
    res.imm0 = info.imm0;
    res.imm1 = info.imm1;
    res.imm0_offset = info.imm0_offset;
    res.imm_count = info.imm_count;

    if (res.group == OpcodeGroup::Control && res.opcode == (uint8_t)ControlOpcode::Closure && res.imm_count >= 2) {
        closure_captures[static_cast<uint32_t>(res.imm0)] = static_cast<uint32_t>(res.imm1);
    }

    return res;
}

void write_int(bytefile *bf, uint32_t word_offset, int32_t value) {
    if (word_offset + sizeof(int32_t) > bf->code_size) {
        fail("internal error: write past code size\n");
    }
    std::memcpy(bf->code_ptr + word_offset, &value, sizeof(int32_t));
}

} 

extern "C" void verify_bytecode(bytefile *bf, const char *output_path) {
    uint32_t code_size = bf->code_size;

    std::unordered_map<uint32_t, uint32_t> closure_captures;

    struct DepthInfo {
        uint32_t depth = 0;
        bool set = false;
    };
    std::unordered_map<State, DepthInfo, StateKeyHash, StateKeyEq> seen;
    std::queue<State> work;
    if (bf->public_symbols_number == 0) {
        work.push(State{0, 0, {}});
    } else {
        uint32_t o = get_public_offset(bf, 0);
        work.push(State{o, 0, {}});
    }

    std::unordered_map<uint32_t, uint32_t> begin_required_depth;

    while (!work.empty()) {
        State st = work.front();
        work.pop();

        Instr ins = decode_instr(bf, st.offset, closure_captures);
        auto it = seen.find(st);
        if (it != seen.end()) {
            if (st.depth <= it->second.depth) {
                continue;
            }
            it->second.depth = st.depth;
        } else {
            seen.emplace(st, DepthInfo{st.depth, true});
        }

        uint32_t next_depth = st.depth;
        auto update_depth = [&](int delta) {
            if (delta < 0) {
                uint32_t d = static_cast<uint32_t>(-delta);
                if (next_depth < d) {
                    throw std::runtime_error("stack underflow at 0x" + std::to_string(st.offset));
                }
                next_depth -= d;
            } else {
                if (next_depth > kMaxDepth - static_cast<uint32_t>(delta)) {
                    throw std::runtime_error("stack overflow at 0x" + std::to_string(st.offset));
                }
                next_depth += static_cast<uint32_t>(delta);
            }
        };

        switch (ins.group) {
        case OpcodeGroup::BinOp:
            update_depth(-1);
            break;
        case OpcodeGroup::Storage:
            switch (static_cast<StorageOpcode>(ins.opcode)) {
            case StorageOpcode::Const:
            case StorageOpcode::String:
                update_depth(1);
                break;
            case StorageOpcode::Sexp:
                update_depth(-ins.imm1 + 1);
                break;
            case StorageOpcode::StoreArray:
                update_depth(-2);
                break;
            case StorageOpcode::Drop:
                update_depth(-1);
                break;
            case StorageOpcode::Dup:
                update_depth(1);
                break;
            case StorageOpcode::Swap:
                break;
            case StorageOpcode::Elem:
                update_depth(-1);
                break;
            case StorageOpcode::End: {
                if (st.frames.empty()) {
                    throw std::runtime_error("END without frame at 0x" + std::to_string(st.offset));
                }
                Frame fr = st.frames.back();
                st.frames.pop_back();
                next_depth = fr.entry_depth + 1;
                break;
            }
            default:
                break;
            }
            break;
        case OpcodeGroup::Load:
            update_depth(1);
            break;
        case OpcodeGroup::LoadAddress:
            update_depth(2);
            break;
        case OpcodeGroup::Store:
            break;
        case OpcodeGroup::Control:
            switch (static_cast<ControlOpcode>(ins.opcode)) {
            case ControlOpcode::JumpIfZero:
            case ControlOpcode::JumpIfNotZero:
                update_depth(-1);
                break;
            case ControlOpcode::Begin: {
                uint32_t args = static_cast<uint32_t>(ins.imm0 & 0xFFFF);
                uint32_t depth_hi = static_cast<uint32_t>((ins.imm0 >> 16) & 0xFFFF);
                uint32_t vars = static_cast<uint32_t>(ins.imm1);
                update_depth(args + vars + kScopeWordCount);
                Frame fr;
                fr.begin_offset = ins.offset;
                fr.entry_depth = st.depth;
                fr.arg_slots = args;
                fr.var_slots = vars;
                fr.captured = 0;
                fr.is_captured = false;
                fr.max_depth = next_depth;
                st.frames.push_back(fr);
                uint32_t delta_alloc = next_depth - fr.entry_depth;
                if (depth_hi != 0 && delta_alloc > depth_hi) {
                    throw std::runtime_error("encoded depth too small at 0x" + std::to_string(ins.offset));
                }
                break;
            }
            case ControlOpcode::BeginCaptured: {
                uint32_t args = static_cast<uint32_t>(ins.imm0 & 0xFFFF);
                uint32_t depth_hi = static_cast<uint32_t>((ins.imm0 >> 16) & 0xFFFF);
                uint32_t vars = static_cast<uint32_t>(ins.imm1);
                auto itc = closure_captures.find(ins.offset);
                if (itc == closure_captures.end()) {
                    throw std::runtime_error("missing capture count for CBEGIN at 0x" + std::to_string(ins.offset));
                }
                uint32_t captured = itc->second;
                update_depth(args + vars + captured + kScopeWordCount + 2);
                Frame fr;
                fr.begin_offset = ins.offset;
                fr.entry_depth = st.depth;
                fr.arg_slots = args;
                fr.var_slots = vars;
                fr.captured = captured;
                fr.is_captured = true;
                fr.max_depth = next_depth;
                st.frames.push_back(fr);
                uint32_t delta_alloc = next_depth - fr.entry_depth;
                if (depth_hi != 0 && delta_alloc > depth_hi) {
                    throw std::runtime_error("encoded depth too small at 0x" + std::to_string(ins.offset));
                }
                break;
            }
            case ControlOpcode::Closure:
                update_depth(1);
                break;
            case ControlOpcode::Fail:
                update_depth(-1);
                break;
            case ControlOpcode::Call:
            case ControlOpcode::CallClosure:
                break;
            default:
                break;
            }
            break;
        case OpcodeGroup::Pattern:
            if (static_cast<PatternOpcode>(ins.opcode) == PatternOpcode::StringMatch) {
                update_depth(-1);
            }
            break;
        case OpcodeGroup::Builtin:
            switch (static_cast<BuiltinOpcode>(ins.opcode)) {
            case BuiltinOpcode::Read:
                update_depth(1);
                break;
            case BuiltinOpcode::MakeArray:
                update_depth(-ins.imm0 + 1);
                break;
            default:
                break;
            }
            break;
        case OpcodeGroup::Halt:
            break;
        }

        if (!st.frames.empty()) {
            Frame &top = st.frames.back();
            if (next_depth > top.max_depth) top.max_depth = next_depth;
            uint32_t extra = top.max_depth - top.entry_depth;
            auto itbd = begin_required_depth.find(top.begin_offset);
            if (itbd == begin_required_depth.end() || extra > itbd->second) {
                begin_required_depth[top.begin_offset] = extra;
            }
        }

        uint32_t next_off = ins.offset + ins.size;
            auto enqueue_state = [&](uint32_t o, uint32_t d, std::vector<Frame> frames) {
                State ns{o, d, std::move(frames)};
                auto it2 = seen.find(ns);
                if (it2 == seen.end() || d > it2->second.depth) {
                    if (it2 == seen.end()) {
                        seen.emplace(ns, DepthInfo{d, true});
                    } else {
                        it2->second.depth = d;
                    }
                    work.push(ns);
                }
            };

        bool allow_fallthrough = !ins.breaks_flow ||
                                 (ins.group == OpcodeGroup::Control &&
                                  (ins.opcode == (uint8_t)ControlOpcode::Call ||
                                   ins.opcode == (uint8_t)ControlOpcode::CallClosure));
        if (allow_fallthrough && next_off < code_size) {
            enqueue_state(next_off, next_depth, st.frames);
        }
        for (uint8_t ti = 0; ti < ins.target_count; ++ti) {
            enqueue_state(ins.targets[ti], next_depth, st.frames);
        }
    }

    for (auto const &kv : begin_required_depth) {
        uint32_t begin_off = kv.first;
        uint32_t extra = kv.second;
        Instr ins = decode_instr(bf, begin_off, closure_captures);
        uint32_t args = static_cast<uint32_t>(ins.imm0 & 0xFFFF);
        if (extra > kMaxDepth) {
            throw std::runtime_error("depth exceeds 65535 at begin 0x" + std::to_string(begin_off));
        }
        int32_t encoded = static_cast<int32_t>((extra << 16) | (args & 0xFFFF));
        write_int(bf, ins.imm0_offset, encoded);
    }

    if (output_path != nullptr) {
        std::ofstream out(output_path, std::ios::binary);
        if (!out) {
            throw std::runtime_error("failed to open output file");
        }
        bytefile_header header;
        header.stringtab_size = bf->stringtab_size;
        header.global_area_size = bf->global_area_size;
        header.public_symbols_number = bf->public_symbols_number;
        out.write(reinterpret_cast<char *>(&header), sizeof(header));
        size_t payload_size = 2U * sizeof(uint32_t) * bf->public_symbols_number +
                              static_cast<size_t>(bf->stringtab_size) + static_cast<size_t>(bf->code_size);
        out.write(bf->buffer, payload_size);
        out.close();
    }
}

#ifndef VERIFY_NO_MAIN
int main(int argc, char *argv[]) {
    if (argc < 2 || argc > 3) {
        std::cerr << "Usage: " << argv[0] << " <input.bc> [output.bc]\n";
        return 1;
    }

    try {
        bytefile *bf = read_file(argv[1]);
        const char *out = (argc == 3) ? argv[2] : nullptr;
        verify_bytecode(bf, out);
        if (out == nullptr) {
            std::cout << "verification succeeded; depths encoded in-memory\n";
        } else {
            std::cout << "written verified bytecode to " << out << "\n";
        }
    } catch (const std::exception &e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
#endif
