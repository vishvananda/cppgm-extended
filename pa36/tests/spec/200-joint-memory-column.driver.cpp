#include <cstdio>
extern "C" void update_memory(unsigned*);
int main() {
    const unsigned inputs[] = {0, 1, 1973, 0x80000000u, 0xffffffffu};
    for (unsigned input : inputs) {
        unsigned objects[] = {0x12345678, input, 0x87654321};
        update_memory(&objects[1]);
        if (objects[1] != input + 200 || objects[0] != 0x12345678 || objects[2] != 0x87654321) return 1;
    }
    std::puts("joint columns select memory with width modifiers and guard bytes passed");
}
