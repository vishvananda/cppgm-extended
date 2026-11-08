#include <cstdio>
extern "C" {
long absolute_target[4] = {11, 13, 17, 19};
const long* asm_absolute_symbol();
const long* asm_absolute_symbol_addend();
}
int main() {
    if (asm_absolute_symbol() != absolute_target) return 1;
    if (asm_absolute_symbol_addend() != absolute_target+2) return 2;
    std::puts("absolute symbol addresses and addends survive source and LowIR transport");
}
