void f(unsigned& value, bool& carry) {
 asm volatile("addl $1, %0; movb %1, %1" : "+r"(value), "=@ccc"(carry));
}
