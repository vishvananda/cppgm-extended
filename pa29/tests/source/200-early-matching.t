extern "C" unsigned long update(unsigned long left, unsigned long right) {
    unsigned long result;
    asm("mov %1, %0; add $17, %0; xor %2, %0"
        : "=&r"(result) : "r"(left), "r"(right) : "cc");
    asm("add $3, %0" : "+r"(result) : : "cc");
    unsigned long copied;
    asm("add $5, %[output]" : [output] "=r"(copied) : "[output]"(result) : "cc");
    return copied;
}
