#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

extern "C" void failure(const char *fmt, ...);

#include "byterun.h"
#include "opcodes.h"

extern "C" {
extern size_t __gc_stack_top, __gc_stack_bottom;
}


extern int32_t *stack_start;
extern int32_t *stack_end;

namespace {

constexpr uint32_t kMaxDepth = 65535;
constexpr uint32_t kScopeWordCount = 10; 

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

static inline void fail_at(uint32_t offset, const char *fmt, ...) {
    char buffer[512];
    int n = std::snprintf(buffer, sizeof(buffer), "bytecode verify error at 0x%x: ", offset);
    if (n < 0) n = 0;
    size_t used = static_cast<size_t>(n);
    if (used >= sizeof(buffer)) used = sizeof(buffer) - 1;
    va_list ap;
    va_start(ap, fmt);
    std::vsnprintf(buffer + used, sizeof(buffer) - used, fmt, ap);
    va_end(ap);
    failure("%s\n", buffer);
}

static inline uint32_t align_up_u32(uint32_t x, uint32_t align) {
    return (x + align - 1) / align * align;
}

static inline void *stack_alloc_bytes(uint32_t bytes, uint32_t offset_for_error) {
    bytes = align_up_u32(bytes, 4);
    uint8_t *top = reinterpret_cast<uint8_t *>(__gc_stack_top);
    uint8_t *new_top = top - bytes;
    if (stack_start == nullptr || stack_end == nullptr) {
        fail_at(offset_for_error, "interpreter stack is not initialized (call verify after Worker::setFile)");
    }
    uint8_t *low = reinterpret_cast<uint8_t *>(stack_start);
    if (new_top < low) {
        fail_at(offset_for_error, "verifier ran out of interpreter stack memory");
    }
    __gc_stack_top = reinterpret_cast<size_t>(new_top);
    return new_top;
}

static inline Instr decode_instr(bytefile *bf, uint32_t offset) {
    instr_info info;
    if (decode_instruction_info(bf, offset, &info) != 0) {
        fail_at(offset, "failed to decode instruction");
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
    return res;
}

static inline void write_int(bytefile *bf, uint32_t word_offset, int32_t value, uint32_t offset_for_error) {
    if (word_offset + sizeof(int32_t) > bf->code_size) {
        fail_at(offset_for_error, "internal error: write past code size");
    }
    std::memcpy(bf->code_ptr + word_offset, &value, sizeof(int32_t));
}

struct ClosureCapture {
    uint32_t entry_offset;
    uint32_t count;
    ClosureCapture *next;
};

static inline uint32_t find_capture_count(ClosureCapture *list, uint32_t entry_offset, uint32_t offset_for_error) {
    for (ClosureCapture *p = list; p != nullptr; p = p->next) {
        if (p->entry_offset == entry_offset) return p->count;
    }
    fail_at(offset_for_error, "missing capture count for CBEGIN (entry=0x%x)", entry_offset);
    return 0;
}

static inline void maybe_update_begin_hint(bytefile *bf, uint32_t begin_offset, uint32_t extra) {
    if (extra > kMaxDepth) {
        fail_at(begin_offset, "required depth exceeds 65535");
    }
    Instr ins = decode_instr(bf, begin_offset);
    if (ins.group != OpcodeGroup::Control) {
        fail_at(begin_offset, "internal error: begin offset does not point to a control opcode");
    }
    uint32_t args = static_cast<uint32_t>(ins.imm0) & 0xFFFFU;
    (void)args;
    uint32_t old_extra = (static_cast<uint32_t>(ins.imm0) >> 16) & 0xFFFFU;
    if (extra <= old_extra) return;
    int32_t encoded = static_cast<int32_t>((extra << 16) | (args & 0xFFFFU));
    write_int(bf, ins.imm0_offset, encoded, begin_offset);
}

struct EntryOffset {
    uint32_t offset;
    EntryOffset *next;
};

static inline bool entry_list_contains(EntryOffset *list, uint32_t off) {
    for (EntryOffset *p = list; p != nullptr; p = p->next) {
        if (p->offset == off) return true;
    }
    return false;
}

struct WorkItem {
    uint32_t offset;
    uint32_t depth;
    WorkItem *next;
};

static void analyze_function(bytefile *bf,
                             uint32_t entry_off,
                             ClosureCapture *captures,
                             uint32_t *best_depth,
                             uint32_t best_len,
                             size_t persistent_top) {
   
    __gc_stack_top = persistent_top;
    for (uint32_t i = 0; i < best_len; ++i) best_depth[i] = 0xFFFFFFFFU;

    uint32_t max_depth = 0;

    WorkItem *work = nullptr;
    auto push_state = [&](uint32_t off, uint32_t depth, uint32_t from_off) {
        if (off >= bf->code_size) {
            fail_at(from_off, "control-flow target out of bounds (0x%x)", off);
        }
        if (depth <= best_depth[off]) return;
        auto *wi = static_cast<WorkItem *>(stack_alloc_bytes(sizeof(WorkItem), from_off));
        wi->offset = off;
        wi->depth = depth;
        wi->next = work;
        work = wi;
    };

    push_state(entry_off, 0, entry_off);

    while (work != nullptr) {
        WorkItem cur = *work;
        work = work->next;

        if (cur.depth <= best_depth[cur.offset]) continue;
        best_depth[cur.offset] = cur.depth;

        Instr ins = decode_instr(bf, cur.offset);
        uint32_t depth = cur.depth;

        auto popn = [&](uint32_t n) {
            if (depth < n) {
                fail_at(cur.offset, "stack underflow");
            }
            depth -= n;
        };
        auto pushn = [&](uint32_t n) {
            if (depth > kMaxDepth - n) {
                fail_at(cur.offset, "stack overflow");
            }
            depth += n;
            if (depth > max_depth) max_depth = depth;
        };

        if (cur.offset != entry_off && ins.group == OpcodeGroup::Control &&
            (ins.opcode == static_cast<uint8_t>(ControlOpcode::Begin) ||
             ins.opcode == static_cast<uint8_t>(ControlOpcode::BeginCaptured))) {
            fail_at(cur.offset, "unexpected BEGIN inside function (jump into callee code?)");
        }

        bool skip_targets = false;

        switch (ins.group) {
        case OpcodeGroup::BinOp:
            popn(2);
            pushn(1);
            break;
        case OpcodeGroup::Storage:
            switch (static_cast<StorageOpcode>(ins.opcode)) {
            case StorageOpcode::Const:
            case StorageOpcode::String:
                pushn(1);
                break;
            case StorageOpcode::Sexp: {
                if (ins.imm1 < 0) fail_at(cur.offset, "negative SEXP count");
                popn(static_cast<uint32_t>(ins.imm1));
                pushn(1);
                break;
            }
            case StorageOpcode::StoreArray:
                popn(3);
                pushn(1);
                break;
            case StorageOpcode::Drop:
                popn(1);
                break;
            case StorageOpcode::Dup:
                popn(1);
                pushn(2);
                break;
            case StorageOpcode::Swap:
                popn(2);
                pushn(2);
                break;
            case StorageOpcode::Elem:
                popn(2);
                pushn(1);
                break;
            case StorageOpcode::End:
                depth = 1;
                if (depth > max_depth) max_depth = depth;
                skip_targets = true;
                break;
            default:
                break;
            }
            break;
        case OpcodeGroup::Load:
            pushn(1);
            break;
        case OpcodeGroup::LoadAddress:
            pushn(2);
            break;
        case OpcodeGroup::Store:
            popn(1);
            pushn(1);
            break;
        case OpcodeGroup::Control:
            switch (static_cast<ControlOpcode>(ins.opcode)) {
            case ControlOpcode::JumpIfZero:
            case ControlOpcode::JumpIfNotZero:
                popn(1);
                break;
            case ControlOpcode::Begin: {
                uint32_t args = static_cast<uint32_t>(ins.imm0) & 0xFFFFU;
                uint32_t vars = static_cast<uint32_t>(ins.imm1);
                uint32_t alloc = args + vars + kScopeWordCount;
                pushn(alloc);
                maybe_update_begin_hint(bf, ins.offset, max_depth);
                break;
            }
            case ControlOpcode::BeginCaptured: {
                uint32_t args = static_cast<uint32_t>(ins.imm0) & 0xFFFFU;
                uint32_t vars = static_cast<uint32_t>(ins.imm1);
                uint32_t captured = find_capture_count(captures, ins.offset, cur.offset);
                uint32_t alloc = args + vars + captured + kScopeWordCount + 2;
                pushn(alloc);
                maybe_update_begin_hint(bf, ins.offset, max_depth);
                break;
            }
            case ControlOpcode::Closure: {
                if (ins.imm1 < 0) fail_at(cur.offset, "negative capture count");
                uint32_t n = static_cast<uint32_t>(ins.imm1);
                for (uint32_t i = 0; i < n; ++i) pushn(1);
                popn(n);
                pushn(1);
                skip_targets = true;
                break;
            }
            case ControlOpcode::Call: {
                if (ins.imm1 < 0) fail_at(cur.offset, "negative CALL argument count");
                uint32_t argc = static_cast<uint32_t>(ins.imm1);
                popn(argc);
                pushn(1);
                skip_targets = true;
                break;
            }
            case ControlOpcode::CallClosure: {
                if (ins.imm0 < 0) fail_at(cur.offset, "negative CALLC argument count");
                uint32_t argc = static_cast<uint32_t>(ins.imm0);
                popn(argc + 1);
                pushn(1);
                skip_targets = true;
                break;
            }
            case ControlOpcode::Fail:
                popn(1);
                skip_targets = true;
                break;
            case ControlOpcode::Tag:
                popn(1);
                pushn(1);
                break;
            case ControlOpcode::Array:
                popn(1);
                pushn(1);
                break;
            case ControlOpcode::Line:
            default:
                break;
            }
            break;
        case OpcodeGroup::Pattern:
            switch (static_cast<PatternOpcode>(ins.opcode)) {
            case PatternOpcode::StringMatch:
                popn(2);
                pushn(1);
                break;
            case PatternOpcode::StringTag:
            case PatternOpcode::ArrayTag:
            case PatternOpcode::Unboxed:
            case PatternOpcode::ClosureTag:
                popn(1);
                pushn(1);
                break;
            default:
                break;
            }
            break;
        case OpcodeGroup::Builtin:
            switch (static_cast<BuiltinOpcode>(ins.opcode)) {
            case BuiltinOpcode::Read:
                pushn(1);
                break;
            case BuiltinOpcode::Write:
            case BuiltinOpcode::Length:
            case BuiltinOpcode::ToString:
                popn(1);
                pushn(1);
                break;
            case BuiltinOpcode::MakeArray: {
                if (ins.imm0 < 0) fail_at(cur.offset, "negative MakeArray length");
                uint32_t n = static_cast<uint32_t>(ins.imm0);
                popn(n);
                pushn(1);
                break;
            }
            default:
                break;
            }
            break;
        case OpcodeGroup::Halt:
            skip_targets = true;
            break;
        }

        if (!skip_targets) {
            const bool allow_fallthrough =
                !ins.breaks_flow ||
                (ins.group == OpcodeGroup::Control &&
                 (ins.opcode == static_cast<uint8_t>(ControlOpcode::Call) ||
                  ins.opcode == static_cast<uint8_t>(ControlOpcode::CallClosure)));
            if (allow_fallthrough && ins.next_offset < bf->code_size) {
                push_state(ins.next_offset, depth, cur.offset);
            }
            for (uint8_t ti = 0; ti < ins.target_count; ++ti) {
                push_state(ins.targets[ti], depth, cur.offset);
            }
        } else {
            const bool allow_fallthrough =
                !ins.breaks_flow ||
                (ins.group == OpcodeGroup::Control &&
                 (ins.opcode == static_cast<uint8_t>(ControlOpcode::Call) ||
                  ins.opcode == static_cast<uint8_t>(ControlOpcode::CallClosure)));
            if (allow_fallthrough && ins.next_offset < bf->code_size) {
                push_state(ins.next_offset, depth, cur.offset);
            }
        }
    }

    
    Instr entry_ins = decode_instr(bf, entry_off);
    if (entry_ins.group == OpcodeGroup::Control &&
        (entry_ins.opcode == static_cast<uint8_t>(ControlOpcode::Begin) ||
         entry_ins.opcode == static_cast<uint8_t>(ControlOpcode::BeginCaptured))) {
        maybe_update_begin_hint(bf, entry_off, max_depth);
    }
}

} // namespace

extern "C" void verify_bytecode(bytefile *bf, const char *output_path) {
    const uint32_t code_size = bf->code_size;
    const size_t saved_top = __gc_stack_top;

    
    ClosureCapture *captures = nullptr;
    EntryOffset *entries = nullptr;

    
    if (bf->public_symbols_number == 0) {
        auto *node = static_cast<EntryOffset *>(stack_alloc_bytes(sizeof(EntryOffset), 0));
        node->offset = 0;
        node->next = entries;
        entries = node;
    } else {
        for (uint32_t i = 0; i < bf->public_symbols_number; ++i) {
            uint32_t off = get_public_offset(bf, i);
            if (!entry_list_contains(entries, off)) {
                auto *node = static_cast<EntryOffset *>(stack_alloc_bytes(sizeof(EntryOffset), off));
                node->offset = off;
                node->next = entries;
                entries = node;
            }
        }
    }

    
    uint32_t off = 0;
    while (off < code_size) {
        Instr ins = decode_instr(bf, off);
        if (ins.group == OpcodeGroup::Control) {
            if (ins.opcode == static_cast<uint8_t>(ControlOpcode::Closure) && ins.imm_count >= 2) {
                uint32_t entry = static_cast<uint32_t>(ins.imm0);
                uint32_t count = static_cast<uint32_t>(ins.imm1);
                bool found = false;
                for (ClosureCapture *p = captures; p != nullptr; p = p->next) {
                    if (p->entry_offset == entry) {
                        found = true;
                        if (p->count != count) {
                            fail_at(off, "inconsistent capture count for entry 0x%x (%u vs %u)", entry, p->count, count);
                        }
                        break;
                    }
                }
                if (!found) {
                    auto *node = static_cast<ClosureCapture *>(stack_alloc_bytes(sizeof(ClosureCapture), off));
                    node->entry_offset = entry;
                    node->count = count;
                    node->next = captures;
                    captures = node;
                }
                if (!entry_list_contains(entries, entry)) {
                    auto *node = static_cast<EntryOffset *>(stack_alloc_bytes(sizeof(EntryOffset), off));
                    node->offset = entry;
                    node->next = entries;
                    entries = node;
                }
            } else if (ins.opcode == static_cast<uint8_t>(ControlOpcode::Call) && ins.imm_count >= 2) {
                uint32_t dst = static_cast<uint32_t>(ins.imm0);
                if (!entry_list_contains(entries, dst)) {
                    auto *node = static_cast<EntryOffset *>(stack_alloc_bytes(sizeof(EntryOffset), off));
                    node->offset = dst;
                    node->next = entries;
                    entries = node;
                }
            }
        }

        if (ins.group == OpcodeGroup::Halt) break;
        if (ins.next_offset <= off || ins.next_offset > code_size) {
            fail_at(off, "invalid next_offset");
        }
        off = ins.next_offset;
    }
    auto *best_depth = static_cast<uint32_t *>(std::malloc(static_cast<size_t>(code_size) * sizeof(uint32_t)));
    if (best_depth == nullptr) {
        failure("bytecode verify error: failed to allocate best_depth\n");
    }
    const size_t persistent_top = __gc_stack_top;

    for (EntryOffset *e = entries; e != nullptr; e = e->next) {
        analyze_function(bf, e->offset, captures, best_depth, code_size, persistent_top);
    }
    std::free(best_depth);

    if (output_path != nullptr) {
        FILE *out = std::fopen(output_path, "wb");
        if (out == nullptr) failure("failed to open output file\n");
        bytefile_header header;
        header.stringtab_size = bf->stringtab_size;
        header.global_area_size = bf->global_area_size;
        header.public_symbols_number = bf->public_symbols_number;
        if (std::fwrite(&header, sizeof(header), 1, out) != 1) failure("failed to write output header\n");
        size_t payload_size = 2U * sizeof(uint32_t) * bf->public_symbols_number +
                              static_cast<size_t>(bf->stringtab_size) + static_cast<size_t>(bf->code_size);
        if (payload_size != 0 && std::fwrite(bf->buffer, 1, payload_size, out) != payload_size) {
            failure("failed to write output payload\n");
        }
        std::fclose(out);
    }

    __gc_stack_top = saved_top;
}
