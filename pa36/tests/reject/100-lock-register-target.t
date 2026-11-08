void rejected(int value) { asm("lock add $1, %0" : "+r"(value)); }
