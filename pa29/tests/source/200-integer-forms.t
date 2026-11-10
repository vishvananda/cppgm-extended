extern "C" unsigned long integer_mix(unsigned long value, unsigned count) {
    asm("imul $-7, %0, %0; rol $13, %0; shr %%cl, %0"
        : "+r"(value) : "c"(count) : "cc");
    return value;
}
extern "C" unsigned long integer_unary(unsigned long value, unsigned* nonzero) {
    bool flag;
    asm("not %0; neg %0; inc %0; dec %0; test %0, %0"
        : "+r"(value), "=@ccnz"(flag) : : "cc");
    *nonzero = flag;
    return value;
}
extern "C" unsigned short integer_bytes(unsigned short value) {
    asm("xchg %b0, %h0" : "+Q"(value));
    return value;
}
