extern "C" __attribute__((naked)) void bad_vex() { asm("lock vaddps %ymm0,%ymm1,%ymm2\nret"); }
