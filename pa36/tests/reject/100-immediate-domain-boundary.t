unsigned bad(unsigned value) {
    asm("add %1, %0" : "+r"(value) : "I"(32) : "cc");
    return value;
}
