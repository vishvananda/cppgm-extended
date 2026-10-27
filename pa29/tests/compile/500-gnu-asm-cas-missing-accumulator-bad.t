void f(unsigned& memory, unsigned expected, unsigned desired) {
 asm volatile("lock; cmpxchgl %2, %1" : "+r"(expected), "+m"(memory) : "r"(desired) : "cc");
}
