# include <stdbool.h>
# include "../runtime32/runtime.h"
# include "byterun.h"
# include "opcodes.h"

# include <stdexcept>
# include <array>
# include <functional>
# include <iostream>
# include <sstream>
# include <vector>
# include <algorithm>
# include <memory>
# include <cstring>
# include <cstdint>
# include <limits>

extern "C" {
    void __pre_gc(void);
    void __post_gc(void);
    void* LmakeArray(int length);
    void* alloc(size_t size);
    void push_extra_root(void **p);
    void pop_extra_root(void **p);
}


const size_t stack_capacity = 65535;
using binop_fn = int32_t (*)(int32_t, int32_t);

#define DEFINE_OP(name, op) \
    static int32_t op_##name(int32_t x, int32_t y) { return x op y; }

DEFINE_OP(add, +)
DEFINE_OP(sub, -)
DEFINE_OP(mul, *)

DEFINE_OP(lt, <)
DEFINE_OP(le, <=)
DEFINE_OP(gt, >)
DEFINE_OP(ge, >=)
DEFINE_OP(eq, ==)
DEFINE_OP(ne, !=)
DEFINE_OP(and, &&)
DEFINE_OP(or, ||)

static int32_t op_div(int32_t x, int32_t y) { if(y == 0){
    throw std::runtime_error("zero div");
}return x / y; }

static int32_t op_mod(int32_t x, int32_t y) { if(y == 0){
    throw std::runtime_error("zero div");
}return x % y; }
using BinOp = ::BinOp;
using OpcodeGroup = ::OpcodeGroup;
using StorageOpcode = ::StorageOpcode;
using AddressMode = ::AddressMode;
using LoadOpcode = ::LoadOpcode;
using ControlOpcode = ::ControlOpcode;
using PatternOpcode = ::PatternOpcode;
using BuiltinOpcode = ::BuiltinOpcode;

struct Sc{
    char* origin_ip;
    Sc* outer;

    int32_t* vars;
    int32_t* args;
    int32_t* acc;
    int32_t var_count;
    int32_t arg_count;
    int32_t acc_count;
    int32_t capture_header;
    int32_t* frame_limit;
};

constexpr size_t scope_word_count = (sizeof(Sc) + sizeof(int32_t) - 1) / sizeof(int32_t);

struct error : std::exception{

    char text[256];

    error(char l, char h){
        sprintf(text, "ERROR: %d-%d\n", h, l);
    }

    error(char const* fmt, ...){
        va_list ap;
        va_start(ap, fmt);
        vsnprintf(text, sizeof text, fmt, ap);
        va_end(ap);
    }

    char const* what() const throw() {return text;}

};
bytefile* bf = nullptr;

Sc* cur_scope = nullptr;
char*  ip        = nullptr;
char*  current_instruction_ptr = nullptr;
char*  code_end  = nullptr;

size_t instruction_bit_number(const char* instruction_ptr) {
    if (bf == nullptr || instruction_ptr == nullptr) {
        throw std::runtime_error("Instruction pointer is not initialized");
    }
    if (code_end == nullptr) {
        throw std::runtime_error("Bytecode stream is not initialized");
    }
    if (instruction_ptr < bf->code_ptr) {
        throw std::runtime_error("Instruction pointer is out of bounds");
    }
    if (instruction_ptr > code_end) {
        throw std::runtime_error("Instruction pointer is out of bounds");
    }
    return static_cast<size_t>(instruction_ptr - bf->code_ptr) * 8;
}
int32_t* stack_start = nullptr;
int32_t* stack_end   = nullptr;
static inline size_t remaining_stack_words() {
    return static_cast<size_t>((__gc_stack_top - reinterpret_cast<size_t>(stack_start)) / sizeof(int32_t));
}
struct Memory{
    std::unique_ptr<int32_t[]> storage;
    int32_t *global_area = nullptr;

    Memory() : storage(new int32_t[stack_capacity]) {}

    int32_t* base() const {
        return storage.get();
    }

    void reset() {
        int32_t* base_ptr = base();
        global_area = nullptr;
        stack_start = base_ptr;
        stack_end = base_ptr + stack_capacity;
        __gc_stack_top = reinterpret_cast<size_t>(stack_end);
        __gc_stack_bottom = reinterpret_cast<size_t>(stack_end);
    }

    int32_t pop() {
        int32_t ret = *reinterpret_cast<int32_t*>(__gc_stack_top);
        __gc_stack_top += sizeof(int32_t);
        return ret;
    }

    void push(void* v){
        push((int32_t)v);
    }

    void push(int32_t v){
        __gc_stack_top -= sizeof(int32_t);
        *reinterpret_cast<int32_t*>(__gc_stack_top) = v;
    }

    void reserve_globals(uint32_t words){
        global_area = base();
        std::fill(global_area, global_area + words, 0);
        stack_start = base() + words;
        stack_end = base() + stack_capacity;
        __gc_stack_top = reinterpret_cast<size_t>(stack_end);
        __gc_stack_bottom = reinterpret_cast<size_t>(stack_end);
    }

    int32_t* globals_ptr() const {
        return global_area;
    }
};

static Sc* allocate_scope(Memory& mem) {
    for (size_t i = 0; i < scope_word_count; ++i) {
        mem.push(0);
    }
    return reinterpret_cast<Sc*>(__gc_stack_top);
}



static inline void ensure_bytes_available(size_t bytes) {
    (void)bytes;
}

static inline int32_t read_int_operand() {
    ensure_bytes_available(sizeof(uint32_t));
    int32_t value;
    std::memcpy(&value, ip, sizeof(value));
    ip += sizeof(uint32_t);
    return static_cast<int32_t>(value);
}

static inline unsigned char read_byte_operand() {
    ensure_bytes_available(1);
    return static_cast<unsigned char>(*ip++);
}

static inline char* read_string_operand() {
    return get_string(bf, static_cast<uint32_t>(read_int_operand()));
}

# define INT    read_int_operand()
# define BYTE   read_byte_operand()
# define STRING read_string_operand()

FILE* log = nullptr;

class Worker{
  public:
    Memory mem;
    bool scope_root_registered = false;

    ~Worker() {
        if (scope_root_registered) {
            pop_extra_root(reinterpret_cast<void**>(&cur_scope));
        }
    }

    size_t current_instruction_bit_number() const {
        if (current_instruction_ptr == nullptr) {
            throw std::runtime_error("Instruction pointer is not initialized");
        }
        return instruction_bit_number(current_instruction_ptr);
    }
    auto& init(){
        __init();

        if (!scope_root_registered) {
            push_extra_root(reinterpret_cast<void**>(&cur_scope));
            scope_root_registered = true;
        }
        cur_scope = nullptr;
        current_instruction_ptr = nullptr;

        mem.reset();

        return *this;
    }
    auto& setFile(bytefile* _bf){
        bf = _bf;
        ip = bf->code_ptr;
        code_end = bf->code_ptr + bf->code_size;
        current_instruction_ptr = nullptr;
        log = stderr;
        mem.reserve_globals(bf->global_area_size);
        bf->global_ptr = mem.globals_ptr();
        return *this;
    }
    int32_t eval(){

        do{
            current_instruction_ptr = ip;
            unsigned char opcode = static_cast<unsigned char>(BYTE);
            char h = static_cast<char>((opcode & 0xF0u) >> 4);
            char l = static_cast<char>(opcode & 0x0Fu);
            auto group = static_cast<OpcodeGroup>(h);
            switch(group){
                case OpcodeGroup::BinOp:        eval_binop(l); break;
                case OpcodeGroup::Storage:      storage(l);    break;
                case OpcodeGroup::Load:
                case OpcodeGroup::LoadAddress:
                case OpcodeGroup::Store:        loading(h, l); break;
                case OpcodeGroup::Control:      control(l);    break;
                case OpcodeGroup::Pattern:      pattern(l);    break;
                case OpcodeGroup::Builtin:      builtins(l);   break;
                case OpcodeGroup::Halt:         return 0;
                default:
                    throw error("unknown opcode group h=%d l=%d at bit %zu",
                                static_cast<int32_t>(h),
                                static_cast<int32_t>(l),
                                current_instruction_bit_number());
            }

        }while(true);
    }
protected:
    void eval_binop(char l){
        int32_t y = UNBOX(mem.pop());
        int32_t x = UNBOX(mem.pop());
        static const binop_fn bops[] = {
            op_add, op_sub, op_mul, op_div, op_mod,
            op_lt, op_le, op_gt, op_ge, op_eq, op_ne,
            op_and, op_or
        };
        
        int32_t res;
        switch (static_cast<BinOp>(l - 1))
        {
            case BinOp::BinOpAdd:
                res = bops[l-1](x, y);
            break;
                
            case BinOp::BinOpSub:
                res = bops[l-1](x, y);
            break;
            
            case BinOp::BinOpMul:
                res = bops[l-1](x, y);
            break;
                
            case BinOp::BinOpDiv: 
                res = bops[l-1](x, y);
            break;
            
            case BinOp::BinOpMod:
                res = bops[l-1](x, y);
            break;
                
            case BinOp::BinOpLt: 
                res = bops[l-1](x, y);
            break;
            
            case BinOp::BinOpLe:
                res = bops[l-1](x, y);
            break;
                
            case BinOp::BinOpGt:
                res = bops[l-1](x, y);
            break;
            
            case BinOp::BinOpGe:
                res = bops[l-1](x, y);
            break;
            
            case BinOp::BinOpEq:
                res = bops[l-1](x, y);
            break;
            
            case BinOp::BinOpNe:
            res = bops[l-1](x, y);
            break;
            
            case BinOp::BinOpAnd:
                res = bops[l-1](x, y);
            break;
            
            case BinOp::BinOpOr:
                res = bops[l-1](x, y);
            break;
            
        
        default:
            throw error("unknown binop group l=%d at bit %zu",
                static_cast<int32_t>(l),
                current_instruction_bit_number());
            break;
        }
        mem.push(BOX(res));
    }
        void storage(char l){
            auto opcode = static_cast<StorageOpcode>(l);
            int32_t res = 0;
            switch (opcode) {
                case StorageOpcode::Const:
                    mem.push(BOX(INT));
                    break;
                case StorageOpcode::String: {
                    int32_t str = (int32_t)(createStr(STRING));
                    mem.push(str);
                    break;
                }
                case StorageOpcode::Sexp: {
                    int32_t hash = LtagHash(STRING);
                    int32_t count = INT;
                    count = ensure_non_negative(count, "sexp element count");
                    mem.push(sexpEval(count, hash));
                    break;
                }
                case StorageOpcode::StoreIndexed:
                    throw error("STI is not supported at bit %zu", current_instruction_bit_number());
                case StorageOpcode::StoreArray: {
                    int32_t v = mem.pop();
                    int32_t i = mem.pop();
                    int32_t loc = mem.pop();
                    mem.push(Bsta((void*)v, i, (void*)loc));
                    break;
                }
                case StorageOpcode::Jump: {
                    int32_t offset = INT;
                    ip = require_code_offset(offset, "jump");
                    break;
                }
                case StorageOpcode::End:{
                    if (cur_scope->outer == nullptr) {
                        exit(0);
                    }
                    res = mem.pop();
                    int32_t* frame_top = cur_scope->frame_limit;
                    __gc_stack_top = reinterpret_cast<size_t>(frame_top);
                    ip = cur_scope->origin_ip;
                    cur_scope = cur_scope->outer;
                    mem.push(res);
                    break;}
                case StorageOpcode::Return:
                    throw error("RET not implemented at bit %zu", current_instruction_bit_number());
                case StorageOpcode::Drop:
                    mem.pop();
                    break;
                case StorageOpcode::Dup: {
                    int32_t v = mem.pop();
                    mem.push(v);
                    mem.push(v);
                    break;
                }
                case StorageOpcode::Swap: {
                    int32_t a = mem.pop();
                    int32_t b = mem.pop();
                    mem.push(b);
                    mem.push(a);
                    break;
                }
                case StorageOpcode::Elem: {
                    int32_t b = mem.pop();
                    int32_t a = mem.pop();
                    mem.push(Belem((void*)a, b));
                    break;
                }
                default:
                    throw error("Unknown storage opcode %d at bit %zu",
                                static_cast<int32_t>(l),
                                current_instruction_bit_number());
            }
        }
    void loading(char h, char l){
            auto mode = static_cast<AddressMode>(l);
            switch (mode) {
                case AddressMode::Global:
                case AddressMode::Local:
                case AddressMode::Argument:
                case AddressMode::Closure:
                    break;
                default:
                    throw error("Unknown address mode %d (opcode %d) at bit %zu",
                                static_cast<int32_t>(l),
                                static_cast<int32_t>(h),
                                current_instruction_bit_number());
            }
            int32_t index = INT;
            int32_t *p = resolve_slot(mode, index);
            switch (static_cast<LoadOpcode>(h)) {
                case LoadOpcode::Load: {
                    int32_t value = *p;
                    mem.push(value);
                    break;
                }
                case LoadOpcode::LoadAddress:
                    mem.push(p);
                    mem.push(p);
                    break;
                case LoadOpcode::Store: {
                    int32_t res = mem.pop();
                    *p = res;
                    mem.push(res);
                    break;
                }
                default:
                    throw error("Unknown load opcode %d at bit %zu",
                                static_cast<int32_t>(h),
                                current_instruction_bit_number());
            }
        }
        void control(char l){
            switch (static_cast<ControlOpcode>(l)) {
                case ControlOpcode::JumpIfZero: {
                    int32_t var = UNBOX(mem.pop());
                    int32_t destination = INT;
                    char* target = require_code_offset(destination, "jump-if-zero target");
                    if (!var) {
                        ip = target;
                    }
                    break;
                }
                case ControlOpcode::JumpIfNotZero: {
                    int32_t var = UNBOX(mem.pop());
                    int32_t destination = INT;
                    char* target = require_code_offset(destination, "jump-if-not-zero target");
                    if (var) {
                        ip = target;
                    }
                    break;
                }
                case ControlOpcode::Begin: {
                    int32_t encoded = INT;
                    int32_t arg_slots = encoded & 0xFFFF;
                    int32_t depth_hint = (encoded >> 16) & 0xFFFF;
                    if (depth_hint > 0) {
                        size_t available = remaining_stack_words();
                        if (static_cast<size_t>(depth_hint) > available) {
                            throw error("Stack depth hint exceeded at bit %zu", current_instruction_bit_number());
                        }
                    }
                    int32_t* frame_entry = reinterpret_cast<int32_t*>(__gc_stack_top);
                    int32_t arg_capacity = arg_slots + 2;
                    int32_t* top_ptr = reinterpret_cast<int32_t*>(__gc_stack_top);
                    top_ptr -= arg_capacity;
                    __gc_stack_top = reinterpret_cast<size_t>(top_ptr);
                    mem.pop(); 
                    char* saved_origin_ip = (char*)mem.pop();
                    int32_t* args_ptr = reinterpret_cast<int32_t*>(__gc_stack_top);
                    int32_t var_slots = INT;
                    top_ptr = reinterpret_cast<int32_t*>(__gc_stack_top);
                    top_ptr -= var_slots;
                    int32_t* vars_ptr = top_ptr;
                    __gc_stack_top = reinterpret_cast<size_t>(top_ptr);
                    Sc* scope = allocate_scope(mem);
                    scope->origin_ip = saved_origin_ip;
                    scope->args = args_ptr;
                    scope->vars = vars_ptr;
                    scope->acc = vars_ptr;
                    scope->var_count = var_slots;
                    scope->arg_count = arg_slots;
                    scope->acc_count = 0;
                    scope->capture_header = 0;
                    scope->frame_limit = frame_entry;
                    scope->outer = cur_scope;
                    cur_scope = scope;
                    break;
                }
                case ControlOpcode::BeginCaptured: {
                    int32_t encoded = INT;
                    int32_t arg_slots = encoded & 0xFFFF;
                    int32_t depth_hint = (encoded >> 16) & 0xFFFF;
                    if (depth_hint > 0) {
                        size_t available = remaining_stack_words();
                        if (static_cast<size_t>(depth_hint) > available) {
                            throw error("Stack depth hint exceeded at bit %zu", current_instruction_bit_number());
                        }
                    }
                    int32_t* frame_entry = reinterpret_cast<int32_t*>(__gc_stack_top);
                    int32_t arg_capacity = arg_slots + 2;
                    int32_t* top_ptr = reinterpret_cast<int32_t*>(__gc_stack_top);
                    top_ptr -= arg_capacity;
                    __gc_stack_top = reinterpret_cast<size_t>(top_ptr);
                    int32_t* args_ptr = top_ptr;
                    int32_t captured = mem.pop();
                    char* saved_origin_ip = (char *)mem.pop();
                    top_ptr = reinterpret_cast<int32_t*>(__gc_stack_top);
                    top_ptr -= 2;
                    __gc_stack_top = reinterpret_cast<size_t>(top_ptr);
                    top_ptr -= captured;
                    int32_t* acc_ptr = top_ptr;
                    __gc_stack_top = reinterpret_cast<size_t>(top_ptr);
                    int32_t var_slots = INT;
                    top_ptr -= var_slots;
                    int32_t* vars_ptr = top_ptr;
                    __gc_stack_top = reinterpret_cast<size_t>(top_ptr);
                    Sc* scope = allocate_scope(mem);
                    scope->origin_ip = saved_origin_ip;
                    scope->args = args_ptr;
                    scope->acc = acc_ptr;
                    scope->vars = vars_ptr;
                    scope->acc_count = captured;
                    scope->arg_count = arg_capacity;
                    scope->var_count = var_slots;
                    scope->capture_header = 2;
                    scope->frame_limit = frame_entry;
                    scope->outer = cur_scope;
                    cur_scope = scope;
                    break;
                }
                case ControlOpcode::Closure: {
                    int32_t res = 0;
                    int32_t pos = INT;
                    (void)require_code_offset(pos, "closure entry");
                    int32_t n = INT;
                    for (int32_t i = 0; i < n; i++) {
                        char tag = BYTE;
                        auto mode = static_cast<AddressMode>(tag);
                        switch (mode) {
                            case AddressMode::Global:
                                res = *require_global_slot(INT);
                                break;
                            case AddressMode::Local:
                                res = *require_local_slot(INT);
                                break;
                            case AddressMode::Argument:
                                res = *require_argument_slot(INT);
                                break;
                            case AddressMode::Closure:
                                res = *require_closure_slot(INT);
                                break;
                            default:
                                throw error("Unknown closure capture mode %d at bit %zu",
                                            static_cast<int32_t>(tag),
                                            current_instruction_bit_number());
                        }
                        mem.push(res);
                    }
                    mem.push(make_closure(n, (void*)pos));
                    break;
                }
                case ControlOpcode::CallClosure: {
                    int32_t n = INT;
                    int32_t* args_ptr = reinterpret_cast<int32_t*>(__gc_stack_top);
                    int32_t* closure_slot = args_ptr + n;
                    data* d = TO_DATA(*closure_slot);
                    if (n > 0) {
                        std::memmove(args_ptr + 1, args_ptr, static_cast<size_t>(n) * sizeof(int32_t));
                    }
                    __gc_stack_top = reinterpret_cast<size_t>(args_ptr + 1);
                    int32_t offset = *(int32_t*)d->contents;
                    int32_t accN = LEN(d->tag) - 1;
                    mem.push(ip);
                    mem.push(accN);
                    for (int32_t i = accN - 1; i >= 0; i--) {
                        mem.push(((int32_t*)d->contents) + i + 1);
                    }
                    ip = require_code_offset(offset, "closure call target");
                    int32_t to_discard = checked_add(accN, checked_add(n, 2, "closure call frame size"), "closure frame discard");
                    for (int32_t i = 0; i < to_discard; ++i) {
                        mem.pop();
                    }
                    break;
                }
                case ControlOpcode::Call: {
                    int32_t v = INT;
                    char* target = require_code_offset(v, "call target");
                    int32_t n = INT;
                    mem.push(ip);
                    mem.push(0);
                    ip = target;
                    int32_t to_discard = checked_add(n, 2, "call frame size");
                    for (int32_t i = 0; i < to_discard; ++i) {
                        mem.pop();
                    }
                    break;
                }
                case ControlOpcode::Tag: {
                    char* s = STRING;
                    int32_t v = LtagHash(s);
                    int32_t n = BOX(INT);
                    int32_t tg = Btag((void*)mem.pop(), v, n);
                    mem.push(tg);
                    break;
                }
                case ControlOpcode::Array: {
                    int32_t n = BOX(INT);
                    int32_t pat = Barray_patt((void*)mem.pop(), n);
                    mem.push(pat);
                    break;
                }
                case ControlOpcode::Fail: {
                    int32_t a = BOX(INT);
                    int32_t b = BOX(INT);
                    Bmatch_failure((void*)mem.pop(), "", a, b);
                    break;
                }
                case ControlOpcode::Line:
                    INT;
                    break;
                default:
                    throw error("Unknown control opcode %d at bit %zu",
                                static_cast<int32_t>(l),
                                current_instruction_bit_number());
            }
        }

        void pattern(char l){
            int32_t res;
            switch (static_cast<PatternOpcode>(l)) {
                case PatternOpcode::StringMatch: {
                    int32_t a = mem.pop();
                    int32_t b = mem.pop();
                    res = Bstring_patt((void*)a, (void*)b);
                    break;
                }
                case PatternOpcode::StringTag:
                    res = Bstring_tag_patt((void *)mem.pop());
                    break;
                case PatternOpcode::ArrayTag:
                    res = Barray_tag_patt((void *)mem.pop());
                    break;
                case PatternOpcode::Unboxed:
                    res = Bunboxed_patt((void *)mem.pop());
                    break;
                case PatternOpcode::ClosureTag:
                    res = Bclosure_tag_patt((void *)mem.pop());
                    break;
                default:
                    throw error("Unknown pattern opcode %d at bit %zu",
                                static_cast<int32_t>(l),
                                current_instruction_bit_number());
            }
            mem.push(res);
        }

        void builtins(char l){
            int32_t res;
            switch (static_cast<BuiltinOpcode>(l)){
                case BuiltinOpcode::Read:
                    res = Lread();
                    break;
                case BuiltinOpcode::Write: {
                    int32_t to_write = mem.pop();
                    res = Lwrite(to_write);
                    break;
                }
                case BuiltinOpcode::Length:
                    res = Llength((void *)mem.pop());
                    break;
                case BuiltinOpcode::ToString:
                    res = (int32_t)Lstring((void *)mem.pop());
                    break;
                case BuiltinOpcode::MakeArray: {
                    int32_t length = INT;
                    res = (int32_t)make_array(length);
                    break;
                }
                default:
                    throw error("Unknown builtin opcode %d at bit %zu",
                                static_cast<int32_t>(l),
                                current_instruction_bit_number());
            }
            mem.push(res);
        }

        void* make_array(int32_t n) {
            void* contents = LmakeArray(BOX(n));
            int32_t* array_data = static_cast<int32_t*>(contents);
            for (int32_t i = n - 1; i >= 0; --i) {
                array_data[i] = mem.pop();
            }
            return contents;
        }

        void* createStr(void *p) {
            const char* source = static_cast<const char*>(p);
            int32_t length = strlen(source);
            char* content = static_cast<char*>(LmakeString(BOX(length)));
            memcpy(content, source, static_cast<size_t>(length) + 1);
            return content;
        }

        void* make_closure (int32_t n, void *entry) {
            __pre_gc();
            int32_t total_slots = checked_add(n, 2, "closure slot count");
            int32_t tag_slots = checked_add(n, 1, "closure tag length");
            data* r = static_cast<data*>(alloc(sizeof(int32_t) * total_slots));
            r->tag = CLOSURE_TAG | (tag_slots << 3);
            ((void**)r->contents)[0] = entry;
            __post_gc();

            for (int32_t i = n - 1; i >= 0; --i) {
                ((int32_t*)r->contents)[i + 1] = mem.pop();
            }
            return r->contents;
        }

        void* sexpEval (int32_t n, int32_t hash) {
            __pre_gc();
            int32_t total_slots = checked_add(n, 2, "sexp slot count");
            sexp* r = static_cast<sexp*>(alloc(sizeof(int32_t) * total_slots));
            data* d = &(r->contents);
            d->tag = SEXP_TAG | (n << 3);
            __post_gc();

            for (int32_t i = n - 1; i >= 0; --i) {
                ((int32_t*)d->contents)[i] = mem.pop();
            }

            r->tag = UNBOX(hash);
            return d->contents;
        }

    private:
        int32_t ensure_non_negative(int32_t value, const char* /*what*/) const {
            return value;
        }

        int32_t checked_add(int32_t lhs, int32_t rhs, const char* /*what*/) const {
            return lhs + rhs;
        }

        Sc* require_scope(const char* /*what*/) const {
            return cur_scope;
        }

        int32_t* require_slot_in_range(int32_t* base,
                                   int32_t /*count*/,
                                   int32_t index,
                                   const char* /*what*/) const {
            return base + index;
        }

        int32_t* require_global_slot(int32_t index) const {
            return bf->global_ptr + index;
        }

        int32_t* require_local_slot(int32_t index) const {
            Sc* scope = cur_scope;
            return scope->vars + index;
        }

        int32_t* require_argument_slot(int32_t index) const {
            Sc* scope = cur_scope;
            int32_t logical_index = scope->arg_count - 1 - index;
            return scope->args + logical_index;
        }

        int32_t* require_closure_slot(int32_t index) const {
            Sc* scope = cur_scope;
            int32_t* slot = scope->acc + index;
            return reinterpret_cast<int32_t*>(*slot);
        }

        int32_t* resolve_slot(AddressMode mode, int32_t index) const {
            switch (mode) {
                case AddressMode::Global:
                    return require_global_slot(index);
                case AddressMode::Local:
                    return require_local_slot(index);
                case AddressMode::Argument:
                    return require_argument_slot(index);
                case AddressMode::Closure:
                    return require_closure_slot(index);
                default:
                    return nullptr;
            }
        }

        char* require_code_offset(int32_t offset, const char* /*what*/) const {
            return bf->code_ptr + offset;
        }
    };
int main(int argc, char*argv[]){
    bytefile* f = read_file(argv[1]);
    Worker w = Worker();
    w.init().setFile(f);
    verify_bytecode(f, nullptr);
    return w.eval();
}
