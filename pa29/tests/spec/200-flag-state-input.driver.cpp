#include <cstdio>
extern "C" void carry_input(unsigned long*);
int main() {
    for (unsigned i = 0; i < 17; ++i) {
        unsigned long out = 0;
        carry_input(&out);
        if (out != 1) return 1;
    }
    std::puts("assembly input setup preserves declared incoming condition flags passed");
}
