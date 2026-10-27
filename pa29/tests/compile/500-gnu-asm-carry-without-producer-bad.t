void f(unsigned& value, bool& carry) {
 asm volatile("movl %0, %0" : "+r"(value), "=@ccc"(carry));
}
