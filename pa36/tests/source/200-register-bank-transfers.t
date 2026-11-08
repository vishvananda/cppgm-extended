extern "C" float flip32(float value) {
    asm("xor $0x80000000, %0" : "+r"(value) : : "cc");
    return value;
}
extern "C" double flip64(double value) {
    asm("xor %1, %0" : "+r"(value) : "r"(1UL << 63) : "cc");
    return value;
}
extern "C" unsigned long mix64(unsigned long left, unsigned long right) {
    asm("pxor %1, %0" : "+x"(left) : "x"(right));
    return left;
}
extern "C" unsigned mix32(unsigned left, unsigned right) {
    asm("pxor %1, %0" : "+x"(left) : "x"(right));
    return left;
}
