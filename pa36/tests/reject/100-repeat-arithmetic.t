void rejected(int value) { asm("rep add $1, %0" : "+r"(value)); }
