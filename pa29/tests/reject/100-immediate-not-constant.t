unsigned bad(unsigned value, unsigned increment) {
    asm("add %1, %0" : "+r"(value) : "n"(increment) : "cc");
    return value;
}
