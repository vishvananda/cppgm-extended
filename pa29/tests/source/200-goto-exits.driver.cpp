#include <cstdio>
extern "C" int goto_select(int);
extern "C" int goto_output(int);
extern "C" int goto_loop(int);
int main() {
    for (int value = -32; value < 33; ++value) {
        if (goto_select(value) != (value < 0 ? 9 : value == 0 ? 8 : 7)) return 1;
        if (goto_output(value) != (value - 1) * (value <= 0 ? 5 : 3)) return 2;
    }
    for (int n = 0; n < 64; ++n) if (goto_loop(n) != n * (n + 1) / 2) return 3;
    std::puts("asm goto named/numeric exits, outputs and backedges passed");
}
