#include <cstdio>
extern "C" long raw_add_23(long);
extern "C" long raw_add_41(long);
extern "C" long raw_external(long);
extern "C" long raw_to_internal(long);
extern "C" long assembly_callback(long value) { return value * 5 + 7; }
int main() {
    for (long x = -101; x < 102; ++x) {
        if (raw_add_23(x) != x + 23) return 1;
        if (raw_add_41(x) != x + 41) return 2;
        if (raw_external(x) != assembly_callback(x)) return 3;
        if (raw_to_internal(x) != x * 2 + 5) return 4;
    }
    std::puts("assembly-defined functions retain opaque ABI and typed constants/symbols");
}
