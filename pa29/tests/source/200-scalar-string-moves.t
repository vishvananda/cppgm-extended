extern "C" float asm_float(float input) {
    float output;
    asm("movss %1, %0" : "=x"(output) : "xm"(input));
    return output;
}
extern "C" double asm_double(double input) {
    double output;
    asm("movsd %1, %0" : "=x"(output) : "xm"(input));
    return output;
}
extern "C" void asm_store_float(float* pointer, float input) {
    asm("movss %1, %0" : "=m"(*pointer) : "x"(input));
}
extern "C" void asm_store_double(double* pointer, double input) {
    asm("movsd %1, %0" : "=m"(*pointer) : "x"(input));
}
extern "C" void asm_copy_words(unsigned* destination, const unsigned* source, unsigned long count) {
    asm volatile("rep movsl" : "+D"(destination), "+S"(source), "+c"(count) : : "memory");
}
extern "C" void asm_set_words(unsigned* destination, unsigned value, unsigned long count) {
    asm volatile("rep stosl" : "+D"(destination), "+c"(count) : "a"(value) : "memory");
}
extern "C" unsigned long asm_zero_move(unsigned* carry) {
    unsigned long value;
    bool result;
    asm("stc; mov $0, %0" : "=r"(value), "=@ccc"(result));
    *carry = result;
    return value;
}
extern "C" unsigned long asm_absolute_move(unsigned* carry) {
    unsigned long value;
    bool result;
    asm("stc; movabs $1, %0" : "=r"(value), "=@ccc"(result));
    *carry = result;
    return value;
}
