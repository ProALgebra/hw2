# include <stdbool.h>
# include "../runtime32/runtime.h"
# include "byterun.h"

# include <stdexcept>
# include <array>
# include <functional>
# include <iostream>
# include <sstream>
# include <vector>
# include <algorithm>
# include <cstring>
# define INT    (ip += sizeof (int), *(int*)(ip - sizeof (int)))
# define BYTE   *ip++
# define STRING get_string (bf, INT)

extern "C" {
    void __pre_gc(void);
    void __post_gc(void);
    void* LmakeArray(int length);
    void* alloc(size_t size);
    void push_extra_root(void **p);
    void pop_extra_root(void **p);
}


const size_t stack_capacity = 1024 * 1024;
using binop_fn = int (*)(int, int);

static int op_add(int x, int y) { return x + y; }
static int op_sub(int x, int y) { return x - y; }
static int op_mul(int x, int y) { return x * y; }
static int op_div(int x, int y) { if(y == 0){
    throw std::runtime_error("zero div");
}return x / y; }
static int op_mod(int x, int y) { if(y == 0){
    throw std::runtime_error("zero div");
}return x % y; }
static int op_lt(int x, int y) { return x < y; }
static int op_le(int x, int y) { return x <= y; }
static int op_gt(int x, int y) { return x > y; }
static int op_ge(int x, int y) { return x >= y; }
static int op_eq(int x, int y) { return x == y; }
static int op_ne(int x, int y) { return x != y; }
static int op_and(int x, int y) { return x && y; }
static int op_or(int x, int y) { return x || y; }

static const binop_fn bops[] = {
    op_add, op_sub, op_mul, op_div, op_mod,
    op_lt, op_le, op_gt, op_ge, op_eq, op_ne,
    op_and, op_or
};

enum class OpcodeGroup : char {
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

enum class StorageOpcode : char {
    Const = 0,
    String = 1,
    Sexp = 2,
    StoreIndexed = 3,
    StoreArray = 4,
    Jump = 5,
    End = 6,
    Return = 7,
    Drop = 8,
    Dup = 9,
    Swap = 10,
    Elem = 11
};

enum class AddressMode : char {
    Global = 0,
    Local = 1,
    Argument = 2,
    Closure = 3
};

enum class LoadOpcode : char {
    Load = 2,
    LoadAddress = 3,
    Store = 4
};

enum class ControlOpcode : char {
    JumpIfZero = 0,
    JumpIfNotZero = 1,
    Begin = 2,
    BeginCaptured = 3,
    Closure = 4,
    CallClosure = 5,
    Call = 6,
    Tag = 7,
    Array = 8,
    Fail = 9,
    Line = 10
};

enum class PatternOpcode : char {
    StringMatch = 0,
    StringTag = 1,
    ArrayTag = 2,
    Unboxed = 5,
    ClosureTag = 6
};

enum class BuiltinOpcode : char {
    Read = 0,
    Write = 1,
    Length = 2,
    ToString = 3,
    MakeArray = 4
};

struct Sc{
    char* origin_ip;
    Sc* outer;

    int* vars;
    int* args;
    int* acc;
};

constexpr size_t scope_word_count = (sizeof(Sc) + sizeof(int) - 1) / sizeof(int);

static Sc* allocate_scope() {
    __pre_gc();
    const int scope_len = static_cast<int>(scope_word_count);
    data* cell = static_cast<data*>(alloc(sizeof(int) * (scope_len + 1)));
    cell->tag = ARRAY_TAG | (scope_len << 3);
    Sc* scope = reinterpret_cast<Sc*>(cell->contents);
    memset(scope, 0, sizeof(Sc));
    __post_gc();
    return scope;
}

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
int* stack_start = nullptr;
int* stack_end   = nullptr;
struct Memory{
    int mem[stack_capacity];
    int *global_area = nullptr;

    int pop() {
        if (__gc_stack_top == reinterpret_cast<size_t>(stack_start)){
            throw std::runtime_error("Pop from empty stack");
        }
        __gc_stack_top -= sizeof(int);
        int ret = *reinterpret_cast<int*>(__gc_stack_top);
        return ret;
    }

    void push(void* v){
        push((int)v);
    }

    void push(int v){
        if (__gc_stack_top == reinterpret_cast<size_t>(stack_end)){
            throw std::runtime_error("Push to full stack");
        }

        *reinterpret_cast<int*>(__gc_stack_top) = v;
        __gc_stack_top += sizeof(int);
    }

    void reserve_globals(int words){
        if (words < 0 || static_cast<size_t>(words) > stack_capacity){
            throw std::runtime_error("Invalid global area size");
        }
        global_area = mem;
        std::fill(global_area, global_area + words, 0);
        int* new_top = mem + words;
        __gc_stack_top = reinterpret_cast<size_t>(new_top);
        stack_start = new_top;
    }

    int* globals_ptr() const {
        return global_area;
    }
};

bytefile* bf = nullptr;

Sc* cur_scope = nullptr;
char*  ip        = nullptr;
char*  current_instruction_ptr = nullptr;

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
    size_t instruction_bit_number(const char* instruction_ptr) const {
        if (bf == nullptr || instruction_ptr == nullptr) {
            throw std::runtime_error("Instruction pointer is not initialized");
        }
        if (instruction_ptr < bf->code_ptr) {
            throw std::runtime_error("Instruction pointer is out of bounds");
        }
        return static_cast<size_t>(instruction_ptr - bf->code_ptr) * 8;
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

        stack_start = mem.mem;
        __gc_stack_top = reinterpret_cast<size_t>(mem.mem);
        __gc_stack_bottom = reinterpret_cast<size_t>(mem.mem);
        stack_end = mem.mem + stack_capacity;

        return *this;
    }
    auto& setFile(bytefile* _bf){
        bf = _bf;
        ip = bf->code_ptr;
        current_instruction_ptr = nullptr;
        log = stderr;
        mem.reserve_globals(bf->global_area_size);
        bf->global_ptr = mem.globals_ptr();
        return *this;
    }
    int eval(){

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
                                static_cast<int>(h),
                                static_cast<int>(l),
                                current_instruction_bit_number());
            }

        }while(true);
    }
protected:
    void eval_binop(char l){
        int y = UNBOX(mem.pop());
        int x = UNBOX(mem.pop());
        int res = bops[l-1](x, y);
        mem.push(BOX(res));
    }
        void storage(char l){
            auto opcode = static_cast<StorageOpcode>(l);
            int res = 0;
            switch (opcode) {
                case StorageOpcode::Const:
                    mem.push(BOX(INT));
                    break;
                case StorageOpcode::String: {
                    int str = (int)(createStr(STRING));
                    mem.push(str);
                    break;
                }
                case StorageOpcode::Sexp: {
                    int hash = LtagHash(STRING);
                    mem.push(sexpEval(INT, hash));
                    break;
                }
                case StorageOpcode::StoreIndexed:
                    throw error("STI is not supported at bit %zu", current_instruction_bit_number());
                case StorageOpcode::StoreArray: {
                    int v = mem.pop();
                    int i = mem.pop();
                    int loc = mem.pop();
                    mem.push(Bsta((void*)v, i, (void*)loc));
                    break;
                }
                case StorageOpcode::Jump:
                    ip = bf->code_ptr + INT;
                    break;
                case StorageOpcode::End:
                    if (cur_scope->outer == nullptr) {
                        exit(0);
                    }
                    res = mem.pop();
                    __gc_stack_top = reinterpret_cast<size_t>(cur_scope->args);
                    ip = cur_scope->origin_ip;
                    cur_scope = cur_scope->outer;
                    mem.push(res);
                    break;
                case StorageOpcode::Return:
                    throw error("RET not implemented at bit %zu", current_instruction_bit_number());
                case StorageOpcode::Drop:
                    mem.pop();
                    break;
                case StorageOpcode::Dup: {
                    int v = mem.pop();
                    mem.push(v);
                    mem.push(v);
                    break;
                }
                case StorageOpcode::Swap: {
                    int a = mem.pop();
                    int b = mem.pop();
                    mem.push(b);
                    mem.push(a);
                    break;
                }
                case StorageOpcode::Elem: {
                    int b = mem.pop();
                    int a = mem.pop();
                    mem.push(Belem((void*)a, b));
                    break;
                }
                default:
                    throw error("Unknown storage opcode %d at bit %zu",
                                static_cast<int>(l),
                                current_instruction_bit_number());
            }
        }
    void loading(char h, char l){
            int *p = nullptr;
            auto mode = static_cast<AddressMode>(l);
            switch (mode) {
                case AddressMode::Global:
                    p = bf->global_ptr + INT;
                    break;
                case AddressMode::Local:
                    p = cur_scope->vars + INT;
                    break;
                case AddressMode::Argument:
                    p = cur_scope->args + INT;
                    break;
                case AddressMode::Closure:
                    p = reinterpret_cast<int*>(*(cur_scope->acc + INT));
                    break;
                default:
                    throw error("Unknown address mode %d (opcode %d) at bit %zu",
                                static_cast<int>(l),
                                static_cast<int>(h),
                                current_instruction_bit_number());
            }
            switch (static_cast<LoadOpcode>(h)) {
                case LoadOpcode::Load: {
                    int value = *p;
                    mem.push(value);
                    break;
                }
                case LoadOpcode::LoadAddress:
                    mem.push(p);
                    mem.push(p);
                    break;
                case LoadOpcode::Store: {
                    int res = mem.pop();
                    *p = res;
                    mem.push(res);
                    break;
                }
                default:
                    throw error("Unknown load opcode %d at bit %zu",
                                static_cast<int>(h),
                                current_instruction_bit_number());
            }
        }
        void control(char l){
            switch (static_cast<ControlOpcode>(l)) {
                case ControlOpcode::JumpIfZero: {
                    int var = UNBOX(mem.pop());
                    int destination = INT;
                    if (!var) {
                        ip = bf->code_ptr + destination;
                    }
                    break;
                }
                case ControlOpcode::JumpIfNotZero: {
                    int var = UNBOX(mem.pop());
                    int destination = INT;
                    if (var) {
                        ip = bf->code_ptr + destination;
                    }
                    break;
                }
                case ControlOpcode::Begin: {
                    Sc* scope = allocate_scope();
                    int* top_ptr = reinterpret_cast<int*>(__gc_stack_top);
                    scope->args = top_ptr;
                    int arg_slots = INT;
                    top_ptr += arg_slots + 1;
                    __gc_stack_top = reinterpret_cast<size_t>(top_ptr);
                    scope->origin_ip = (char*)mem.pop();
                    top_ptr = reinterpret_cast<int*>(__gc_stack_top);
                    scope->vars = top_ptr;
                    scope->acc = top_ptr;
                    int var_slots = INT;
                    top_ptr += var_slots;
                    __gc_stack_top = reinterpret_cast<size_t>(top_ptr);
                    scope->outer = cur_scope;
                    cur_scope = scope;
                    break;
                }
                case ControlOpcode::BeginCaptured: {
                    Sc* scope = allocate_scope();
                    int arg_slots = INT;
                    int* top_ptr = reinterpret_cast<int*>(__gc_stack_top);
                    scope->args = top_ptr;
                    top_ptr += arg_slots + 2;
                    __gc_stack_top = reinterpret_cast<size_t>(top_ptr);
                    int captured = mem.pop();
                    scope->origin_ip = (char *)mem.pop();
                    top_ptr = reinterpret_cast<int*>(__gc_stack_top);
                    top_ptr += 2;
                    __gc_stack_top = reinterpret_cast<size_t>(top_ptr);
                    scope->acc = top_ptr;
                    top_ptr += captured;
                    __gc_stack_top = reinterpret_cast<size_t>(top_ptr);
                    scope->vars = top_ptr;
                    int var_slots = INT;
                    top_ptr += var_slots;
                    __gc_stack_top = reinterpret_cast<size_t>(top_ptr);
                    scope->outer = cur_scope;
                    cur_scope = scope;
                    break;
                }
                case ControlOpcode::Closure: {
                    int res = 0;
                    int pos = INT;
                    int n = INT;
                    for (int i = 0; i < n; i++) {
                        char tag = BYTE;
                        switch (static_cast<AddressMode>(tag)) {
                            case AddressMode::Global:
                                res = *(bf->global_ptr + INT);
                                break;
                            case AddressMode::Local:
                                res = *(cur_scope->vars + INT);
                                break;
                            case AddressMode::Argument:
                                res = *(cur_scope->args + INT);
                                break;
                            case AddressMode::Closure:
                                res = *(int*)*(cur_scope->acc + INT);
                                break;
                            default:
                                throw error("Unknown closure capture mode %d at bit %zu",
                                            static_cast<int>(tag),
                                            current_instruction_bit_number());
                        }
                        mem.push(res);
                    }
                    mem.push(make_closure(n, (void*)pos));
                    break;
                }
                case ControlOpcode::CallClosure: {
                    int n = INT;
                    int* top_ptr = reinterpret_cast<int*>(__gc_stack_top);
                    data* d = TO_DATA(*(top_ptr - n - 1));
                    memmove(top_ptr - n - 1, top_ptr - n, n * sizeof(int));
                    top_ptr -= 1;
                    __gc_stack_top = reinterpret_cast<size_t>(top_ptr);
                    int offset = *(int*)d->contents;
                    int accN = LEN(d->tag) - 1;
                    mem.push(ip);
                    mem.push(accN);
                    for (int i = accN - 1; i >= 0; i--) {
                        mem.push(((int*)d->contents) + i + 1);
                    }
                    ip = bf->code_ptr + offset;
                    top_ptr = reinterpret_cast<int*>(__gc_stack_top);
                    top_ptr -= accN + n + 2;
                    __gc_stack_top = reinterpret_cast<size_t>(top_ptr);
                    break;
                }
                case ControlOpcode::Call: {
                    int v = INT;
                    int n = INT;
                    mem.push(ip);
                    mem.push(0);
                    ip = bf->code_ptr + v;
                    int* top_ptr = reinterpret_cast<int*>(__gc_stack_top);
                    top_ptr -= (n + 2);
                    __gc_stack_top = reinterpret_cast<size_t>(top_ptr);
                    break;
                }
                case ControlOpcode::Tag: {
                    char* s = STRING;
                    int v = LtagHash(s);
                    int n = BOX(INT);
                    int tg = Btag((void*)mem.pop(), v, n);
                    mem.push(tg);
                    break;
                }
                case ControlOpcode::Array: {
                    int n = BOX(INT);
                    int pat = Barray_patt((void*)mem.pop(), n);
                    mem.push(pat);
                    break;
                }
                case ControlOpcode::Fail: {
                    int a = BOX(INT);
                    int b = BOX(INT);
                    Bmatch_failure((void*)mem.pop(), "", a, b);
                    break;
                }
                case ControlOpcode::Line:
                    INT;
                    break;
                default:
                    throw error("Unknown control opcode %d at bit %zu",
                                static_cast<int>(l),
                                current_instruction_bit_number());
            }
        }

        void pattern(char l){
            int res;
            switch (static_cast<PatternOpcode>(l)) {
                case PatternOpcode::StringMatch: {
                    int a = mem.pop();
                    int b = mem.pop();
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
                                static_cast<int>(l),
                                current_instruction_bit_number());
            }
            mem.push(res);
        }

        void builtins(char l){
            int res;
            switch (static_cast<BuiltinOpcode>(l)){
                case BuiltinOpcode::Read:
                    res = Lread();
                    break;
                case BuiltinOpcode::Write: {
                    int to_write = mem.pop();
                    res = Lwrite(to_write);
                    break;
                }
                case BuiltinOpcode::Length:
                    res = Llength((void *)mem.pop());
                    break;
                case BuiltinOpcode::ToString:
                    res = (int)Lstring((void *)mem.pop());
                    break;
                case BuiltinOpcode::MakeArray:
                    res = (int)make_array(INT);
                    break;
                default:
                    throw error("Unknown builtin opcode %d at bit %zu",
                                static_cast<int>(l),
                                current_instruction_bit_number());
            }
            mem.push(res);
        }

        void* make_array(int n) {
            void* contents = LmakeArray(BOX(n));
            int* array_data = static_cast<int*>(contents);
            for (int i = n - 1; i >= 0; --i) {
                array_data[i] = mem.pop();
            }
            return contents;
        }

        void* createStr(void *p) {
            const char* source = static_cast<const char*>(p);
            int length = strlen(source);
            char* content = static_cast<char*>(LmakeString(BOX(length)));
            memcpy(content, source, static_cast<size_t>(length) + 1);
            return content;
        }

        void* make_closure (int n, void *entry) {
            __pre_gc();
            data* r = static_cast<data*>(alloc(sizeof(int) * (n + 2)));
            r->tag = CLOSURE_TAG | ((n + 1) << 3);
            ((void**)r->contents)[0] = entry;
            __post_gc();

            for (int i = 0; i < n; ++i) {
                ((int*)r->contents)[i + 1] = mem.pop();
            }
            return r->contents;
        }

        void* sexpEval (int n, int hash) {
            __pre_gc();
            sexp* r = static_cast<sexp*>(alloc(sizeof(int) * (n + 2)));
            data* d = &(r->contents);
            d->tag = SEXP_TAG | (n << 3);
            __post_gc();

            for (int i = n - 1; i >= 0; --i) {
                ((int*)d->contents)[i] = mem.pop();
            }

            r->tag = UNBOX(hash);
            return d->contents;
        }

    };
int main(int argc, char*argv[]){
    Worker w = Worker();
    return w.init().setFile(read_file(argv[1])).eval();
}
