#include <cstdio>
extern "C" long naked_virtual();
extern "C" long naked_template(long);
extern "C" long host_raw_copy(long) asm("_Z12raw_templateIlElT_");
extern "C" long host_raw_copy(long value) { return value + 9; }
int main() {
    if (naked_virtual() != 42) return 1;
    if (host_raw_copy(13) != 22) return 3;
    for (long value = -71; value < 72; ++value)
        if (naked_template(value) != value + 24) return 2;
    std::puts("naked virtual adjustment and instantiated ABI passed");
}
