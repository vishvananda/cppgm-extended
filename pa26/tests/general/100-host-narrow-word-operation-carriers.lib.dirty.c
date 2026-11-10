/* Native-only helpers put legal garbage in bits above a 32-bit argument.
   Each wrapper aligns the call stack and extends the result's own ABI bits. */
#ifdef __APPLE__
#define SYMBOL_PREFIX "_"
#else
#define SYMBOL_PREFIX ""
#endif
#define WRAPPER(NAME, TARGET, EXTEND) \
__attribute__((naked)) long NAME(unsigned long a, unsigned long b) { \
    __asm__ volatile("subq $8, %rsp\ncall " SYMBOL_PREFIX #TARGET "\n" \
                     EXTEND "\naddq $8, %rsp\nret\n"); \
}
WRAPPER(dirty_sd, cpp_sd, "cltq")
WRAPPER(dirty_sm, cpp_sm, "cltq")
WRAPPER(dirty_ud, cpp_ud, "movl %eax, %eax")
WRAPPER(dirty_um, cpp_um, "movl %eax, %eax")
WRAPPER(dirty_cs, cpp_cs, "cltq")
WRAPPER(dirty_cu, cpp_cu, "movl %eax, %eax")
WRAPPER(dirty_power, cpp_power, "movl %eax, %eax")
WRAPPER(dirty_add, cpp_add, "cltq")
WRAPPER(dirty_right, cpp_right, "cltq")
WRAPPER(dirty_uright, cpp_uright, "movl %eax, %eax")
