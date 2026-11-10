#include <cstdio>
extern "C" unsigned immediate(unsigned);
int main() {
    const unsigned delta = 23u + 31u + 63u - 128u + 255u + 3u + 255u + 0x80000000u + 0xffffffffu - 1u + 500u;
    const unsigned inputs[] = {0, 1, 1973, 0x80000000u, 0xffffffffu};
    for (unsigned input : inputs) {
        if (immediate(input) != input + delta) return 1;
    }
    std::puts("constant expressions, modifiers and immediate domains passed");
}
