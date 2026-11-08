#include <cstdio>
extern "C" long mixed_dialect(long);
extern "C" long mixed_raw(long);
int main() {
    for (long value=-63;value<64;++value)
        if (mixed_dialect(value)!=(value<0?-value:value)+21 || mixed_raw(value)!=value+12) return 1;
    std::puts("dialect switches preserve numeric branches and register-prefix modes");
}
