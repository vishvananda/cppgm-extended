extern "C" __attribute__((naked)) void bad_vex() { asm("vpshufd $256,%ymm1,%ymm0\nret"); }
