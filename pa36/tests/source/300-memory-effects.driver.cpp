#include <cstdio>
extern "C" unsigned effects(unsigned, unsigned);
int main() {
    const unsigned inputs[] = {0, 1, 1973, 0x80000000u, 0xffffffffu};
    for (unsigned left : inputs) {
        for (unsigned right : inputs) {
            const unsigned expected = left ^ right ^ 23u ^ (left + 17u) ^ (right + 17u);
            if (effects(left, right) != expected) return 1;
        }
    }
    std::puts("opaque memory effects, aliases and inlined assembly passed");
}
