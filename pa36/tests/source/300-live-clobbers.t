typedef float Vector __attribute__((vector_size(16)));
extern "C" unsigned long live(unsigned long a, unsigned long b, unsigned long c,
    unsigned long d, unsigned long e, unsigned long f, unsigned long g,
    unsigned long h, Vector input, Vector* output) {
    asm volatile("xorq %%rax, %%rax; movabsq $0x1122334455667788, %%r12;"
        "xorq %%rdx, %%rdx; xorq %%r8, %%r8; xorq %%r9, %%r9;"
        "pxor %%xmm0, %%xmm0; pxor %%xmm6, %%xmm6"
        : : : "rax", "r12", "rdx", "r8", "r9", "xmm0", "xmm6", "memory", "cc");
    *output = input;
    return (a * 3 + b * 5) ^ (c * 7 + d * 11) ^ (e * 13 + f * 17) ^ (g * 19 + h * 23);
}
