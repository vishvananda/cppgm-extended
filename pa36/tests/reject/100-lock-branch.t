void rejected() { asm volatile("lock jmp 1f; 1: nop"); }
