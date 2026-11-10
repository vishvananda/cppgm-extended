#include <cstdio>
extern "C" void naked_unwind_call(void (*)(), long);
volatile unsigned long seed = 0x4a739c812de5f60bul;
int callbacks = 0;
void normal_callback() { ++callbacks; }
void throwing_callback() { ++callbacks; throw 73; }
int main() {
    register unsigned long live asm("rbx") = seed;
    asm volatile("" : "+r"(live));
    naked_unwind_call(normal_callback, 1);
    if (callbacks != 0) return 1;
    naked_unwind_call(normal_callback, 0);
    asm volatile("" : "+r"(live));
    if (callbacks != 1 || live != seed) return 2;
    bool caught = false;
    try { naked_unwind_call(throwing_callback, 0); }
    catch (int value) { caught = value == 73; }
    asm volatile("" : "+r"(live));
    if (!caught || callbacks != 2 || live != seed) return 3;
    std::puts("module CFI, local branches, alternate epilogues and exception register recovery passed");
}
