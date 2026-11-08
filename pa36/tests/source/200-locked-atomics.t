extern "C" int locked_add(int* location, int delta) {
    asm volatile("lock xadd %0, %1" : "+r"(delta), "+m"(*location) : : "memory", "cc");
    return delta;
}
extern "C" int locked_compare(int* location, int expected, int desired) {
    asm volatile("lock cmpxchg %2, %1" : "+a"(expected), "+m"(*location) : "r"(desired) : "memory", "cc");
    return expected;
}
