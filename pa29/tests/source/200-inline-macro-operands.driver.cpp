#include <cstdio>
extern "C" long macro_storage;
extern "C" long macro_inline_add(long,long);
extern "C" long macro_raw_add(long,long);
extern "C" int macro_inline_goto(long);
extern "C" long* macro_inline_address();
int main() {
    for (long a=-41;a<42;++a) {
        for (long b=-13;b<14;++b)
            if (macro_inline_add(a,b)!=a+b || macro_raw_add(a,b)!=a+b) return 1;
        if (macro_inline_goto(a)!=(a<0?7:3)) return 2;
    }
    if (macro_inline_address()!=&macro_storage || *macro_inline_address()!=37) return 3;
    std::puts("module macros preserve inline registers, symbols, outgoing labels and raw bodies");
}
