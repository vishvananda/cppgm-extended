#include <cstdio>
extern "C" void change_value(const unsigned*, unsigned*);
int main() {
    const unsigned inputs[] = {0, 1, 1973, 0x80000000u, 0xffffffffu};
    for (unsigned original : inputs) {
        unsigned input = original, output = 0;
        change_value(&input, &output);
        if (input != original || output != original + 17) return 1;
    }
    std::puts("distinct input storage and output assignment passed");
}
