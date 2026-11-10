#include <cstdio>
extern "C" unsigned mixed(unsigned);
int main() {
    const unsigned inputs[] = {0, 1, 1973, 0x80000000u, 0xffffffffu};
    for (unsigned input : inputs) {
        if (mixed(input) != input + 15) return 1;
    }
    std::puts("joint constraint columns and mixed locations passed");
}
