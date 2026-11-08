#include <cstdio>
extern "C" long raw_macro_first(long);
extern "C" long raw_macro_second(long);
int main() {
    for (long value=-73;value<74;++value)
        if (raw_macro_first(value)!=value+11 || raw_macro_second(value)!=value+23) return 1;
    std::puts("module and raw macro expansions retain distinct unique labels");
}
