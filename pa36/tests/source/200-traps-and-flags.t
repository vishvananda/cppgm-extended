extern "C" void encoded_traps() {
    asm volatile("int $7; int $3; int3; int1; syscall; endbr64" ::: "memory", "flags", "rax", "rcx", "r11");
}
extern "C" unsigned flags_alias(unsigned value) {
    unsigned char carry;
    asm("addl $1, %0; setc %1" : "+r"(value), "=qm"(carry) : : "flags");
    return value + carry;
}
