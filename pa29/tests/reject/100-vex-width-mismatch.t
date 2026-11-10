extern "C" __attribute__((naked)) void bad_vex() { asm("vaddps %ymm0,%xmm1,%ymm2\nret"); }
