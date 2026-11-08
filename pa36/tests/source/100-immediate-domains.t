extern "C" unsigned immediate(unsigned value) {
    constexpr int folded = 2 + 3;
    asm("add %1, %0; add $%c2, %0; sub $%n3, %0"
        : "+r"(value) : "i"(folded), "n"(7), "i"(11) : "cc");
    asm("add %1, %0; add %2, %0; add %3, %0; add %4, %0"
        : "+r"(value) : "I"(31), "J"(63), "K"(-128), "L"(255) : "cc");
    asm("add %1, %0; add %2, %0; add %3, %0; add %4, %0"
        : "+r"(value) : "M"(3), "N"(255), "e"(-2147483647 - 1), "Z"(0xffffffffULL) : "cc");
    asm("add %1, %0" : "+r"(value) : "K"(~0u) : "cc");
#ifdef __clang__
    asm("add %1, %0" : "+r"(value) : "i"(500) : "cc");
#else
    asm("add %1, %0" : "+r"(value) : "Ki"(500) : "cc");
#endif
    return value;
}
