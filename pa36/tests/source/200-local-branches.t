static inline unsigned sum(unsigned value) {
    unsigned result;
    asm("mov $0, %[result]\n\t"
        "cmp $0, %[value]\n\t"
        "je .Ldone%=\n\t"
        "1: add %[value], %[result]\n\t"
        "sub $1, %[value]\n\t"
        "jne 1b\n\t"
        "jmp 2f\n\t"
        "1: add $99, %[result]\n\t"
        "2: .Ldone%=:"
        : [result] "=&r"(result), [value] "+r"(value) : : "cc");
    return result;
}
extern "C" unsigned branch_sum(unsigned a, unsigned b) {
    return sum(a) + sum(b);
}
