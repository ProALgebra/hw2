# include <stdbool.h>
# include "../runtime32/runtime.h"
# include "byterun.h"

# include <stdexcept>
# include <array>
# include <functional>
# include <iostream>
# include <sstream>
# include <vector>
# define INT    (ip += sizeof (int), *(int*)(ip - sizeof (int)))
# define BYTE   *ip++
# define STRING get_string (bf, INT)

extern "C" {
    void __pre_gc(void);
    void __post_gc(void);
    void* LmakeArray(int length);
    void* alloc(size_t size);
}


const size_t stack_capacity = 1024 * 1024;
using binop_fn = int (*)(int, int);

static int op_add(int x, int y) { return x + y; }
static int op_sub(int x, int y) { return x - y; }
static int op_mul(int x, int y) { return x * y; }
static int op_div(int x, int y) { return x / y; }
static int op_mod(int x, int y) { return x % y; }
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

struct Sc{
    char* origin_ip;
    Sc* outer;

    int* vars;
    int* args;
    int* acc;
};

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
    int *stac_top;

    int pop() {
        if (stac_top == stack_start){
            throw std::runtime_error("Pop from empty stack");
        }
        stac_top--;
        __gc_stack_top = (size_t)stac_top;
        int ret = *stac_top;
        return ret;
    }

    void push(void* v){
        push((int)v);
    }

    void push(int v){
        if (stac_top == stack_end){
            throw std::runtime_error("Push to full stack");
        }

        *stac_top = v;
        stac_top++;
        __gc_stack_top = (size_t)stac_top;
    }
};

bytefile* bf = nullptr;

Sc* cur_scope = nullptr;
char*  ip        = nullptr;

FILE* log = nullptr;

class Worker{
  public:
    Memory mem;
    auto& init(){
        __init();

        mem.stac_top = mem.mem;
        stack_start = mem.stac_top;
        __gc_stack_bottom = (size_t)mem.stac_top;
        stack_end = mem.stac_top + stack_capacity;

        return *this;
    }
    auto& setFile(bytefile* _bf){
        bf = _bf;
        ip = bf->code_ptr;
        log = stderr;
        return *this;
    }
    int eval(){

        do{
            char x = BYTE,
                 h = (x & 0xF0) >> 4,
                 l = x & 0x0F;
            switch(h){
                case 0: eval_binop(l);    break;
                case 1: storage(l);     break;
                case 2:
                case 3:
                case 4: loading(h, l);  break;
                case 5: control(l);  break;
                case 6: pattern(l);  break;
                case 7: builtins(l); break;
                case 15: return 0;
                default: throw error(h, l);
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
            int res;
            switch (l) {
                    case 0: mem.push(BOX(INT));                                                    break;
                    case 1:{
                        int res = (int)createStr(STRING);
                        mem.push(res);
                    }      break;                                                                          break;
                    case 2:{
                        int hash = LtagHash(STRING);
                        mem.push(sexpEval(INT, hash));
                    }         break;                                                                       break;
                    case 3: throw error("STI is not supported");
                    case 4: {
                        int v = mem.pop();
                        int i = mem.pop();
                        int loc = mem.pop();
                        mem.push(Bsta((void*)v, i, (void*)loc));
                    }       break;                                                                         break;
                    case 5: ip = bf->code_ptr + INT;                                                 break;
                    case 6:{
                        if (cur_scope->outer == nullptr) {
                            exit(0);
                        }
                        res = mem.pop();
                        mem.stac_top = cur_scope->args;
                        ip = cur_scope->origin_ip;
                        cur_scope = cur_scope->outer;
                        mem.push(res);
                    }       break;                                                                         break;
                    case  7: throw error("RET not implemented");
                    case  8: mem.pop();                                                            break;
                    case  9:{int v = mem.pop(); mem.push(v); mem.push(v);}                     break;
                    case 10:{int a = mem.pop();int b = mem.pop(); mem.push(b); mem.push(a);} break;
                    case 11:{
                        int b = mem.pop();
                        int a = mem.pop();
                        mem.push(Belem((void*)a, b));
                    }      break;                                                                          break;
                    default: throw error("Bad instructin %d", l);
            }
        }
    void loading(char h, char l){

            int *p = 0;
            switch (l) {
                case 0: p = bf->global_ptr  + INT;            break;
                case 1: p = cur_scope->vars + INT;            break;
                case 2: p = cur_scope->args + INT;            break;
                case 3: p = (int*)(*(cur_scope->acc + INT));  break;
                default: throw error(h, l);
            }
            switch(h){
                case 2: { //LD
                  int value = *p; mem.push(value);}
                    break;
                case 3:  //LDA
                  mem.push(p); mem.push(p);
                  break;
                case 4: {//ST
                  int res = mem.pop();
                  *p = res; mem.push(res);}
                break;
                default: throw error(h, l);
            }
        }
        void control(char l){
            switch (l) {
                case 0: {
                  int var = UNBOX(mem.pop());
                  int distantion = INT;
                  if (!var)ip = bf->code_ptr + distantion;}
                break;
                case 1: {
                  int var = UNBOX(mem.pop());
                  int distantion = INT;
                  if ( var)ip = bf->code_ptr + distantion;}
                break;
                case 2: {
                    Sc* scope = (Sc*)malloc(sizeof(Sc));
                    scope->args = mem.stac_top;
                    mem.stac_top += INT + 1;
                    scope->origin_ip = (char*)mem.pop();
                    scope->vars = mem.stac_top;
                    scope->acc = scope->vars;
                    mem.stac_top += INT;
                    scope->outer = cur_scope;
                    cur_scope = scope;
                } break;
                case 3:{
                    Sc* scope = (Sc*)malloc(sizeof(Sc));
                    int n = INT;
                    scope->args = mem.stac_top;
                    mem.stac_top += n + 2;
                    n = mem.pop();
                    scope->origin_ip = (char *)mem.pop();
                    mem.stac_top += 2;
                    scope->acc = mem.stac_top;
                    mem.stac_top += n;
                    scope->vars = mem.stac_top;
                    mem.stac_top += INT;
                    scope->outer = cur_scope;
                    cur_scope = scope;
                } break;
                case  4:{
                    int res = 0;
                    int pos = INT;
                    int n = INT;
                    for (int i = 0; i < n; i++) {
                        switch (BYTE) {
                            case 0: res = *(bf->global_ptr + INT);        break;
                            case 1: res = *(cur_scope->vars + INT);       break;
                            case 2: res = *(cur_scope->args + INT);       break;
                            case 3: res = *(int*)*(cur_scope->acc + INT); break;
                            default: throw error(" error in closure");
                        }
                        mem.push(res);
                    }
                    mem.push(make_closure(n, (void*)pos));

                } break;
                case  5:{//CALLC
                    int n = INT;
                    data* d = TO_DATA(*(mem.stac_top - n - 1));
                    memmove(mem.stac_top - n - 1, mem.stac_top - n, n * sizeof(int));
                    mem.stac_top--;
                    int offset = *(int*)d->contents;
                    int accN = LEN(d->tag) - 1;
                    mem.push(ip);
                    mem.push(accN);
                    for (int i = accN - 1; i >= 0; i--) {
                        mem.push(((int*)d->contents) + i + 1);
                    }
                    ip = bf->code_ptr + offset;
                    mem.stac_top -= accN+n+2;
                } break;
                case 6 :{
                    int v = INT;
                    int n = INT;
                    mem.push(ip);
                    mem.push(0);
                    ip = bf->code_ptr + v;
                    mem.stac_top -= (n + 2);
                } break;
                case 7 :{
                    char* s = STRING;
                    int v = LtagHash(s);
                    int n = BOX(INT);
                    int tg = Btag((void*)mem.pop(), v, n);
                    mem.push(tg);
                } break;
                case 8 :{
                    int n = BOX(INT);
                    int pat = Barray_patt((void*)mem.pop(), n);
                    mem.push(pat);
                } break;
                case 9 :{
                    int a = BOX(INT);
                    int b = BOX(INT);
                    Bmatch_failure((void*)mem.pop(), "", a, b);
                } break;
                case 10: INT;                                                                            break;
                default: throw error("Bad on: %d", l);
            }
        }

        void pattern(char l){
            int res;
            switch (l) {
                case 0: {
                    int a = mem.pop();
                    int b = mem.pop();
                    res = Bstring_patt((void*)a, (void*)b);
                } break;
                case 1: res = Bstring_tag_patt((void *)mem.pop());                  break;
                case 2: res = Barray_tag_patt((void *)mem.pop());                   break;
                case 5: res = Bunboxed_patt((void *)mem.pop());                     break;
                case 6: res = Bclosure_tag_patt((void *)mem.pop());                 break;
                default: throw error("Bad on %d", l);
            }
            mem.push(res);
        }

        void builtins(char l){
            int res;
            switch (l){
                case 0: {
                    res = Lread();
                }
                break;
                case 1: {
                    int to_write = mem.pop();
                    res = Lwrite(to_write);
                }                                                 
                break;
                case 2:
                  res = Llength((void *)mem.pop());       
                  break;
                case 3: 
                  res = (int)Lstring((void *)mem.pop());  
                  break;
                case 4: res = 
                      (int)make_array(INT);              
                      break;
                default: throw error("Bad on: %d", l);
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