typedef float Vector __attribute__((vector_size(16)));
extern "C" Vector complement(Vector input) {
    Vector output;
    asm("pcmpeqb %0, %0; pxor %1, %0" : "=&x"(output) : "x"(input));
    return output;
}
