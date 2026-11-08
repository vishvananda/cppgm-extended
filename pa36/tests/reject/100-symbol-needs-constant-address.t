void rejected(int* pointer) { asm volatile("nop" : : "i"(pointer)); }
