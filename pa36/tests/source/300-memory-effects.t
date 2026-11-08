static inline unsigned bump(unsigned value) {
    asm("addl %1, %0" : "+r"(value) : "i"(17) : "cc");
    return value;
}
extern "C" unsigned effects(unsigned left, unsigned right) {
    unsigned local = left;
    const unsigned before = local;
    asm volatile("movl %1, %0" : "=m"(local) : "r"(right));
    const unsigned middle = local;
    asm volatile("movl $23, (%0)" : : "r"(&local) : "memory");
    return before ^ middle ^ local ^ bump(left) ^ bump(right);
}
