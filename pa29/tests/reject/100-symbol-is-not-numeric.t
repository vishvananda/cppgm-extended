extern int value;
void rejected() { asm volatile("nop" : : "n"(&value)); }
