void f(unsigned& value) {
 asm volatile("1: movl %0, %0; jne 1b" : "+r"(value));
}
