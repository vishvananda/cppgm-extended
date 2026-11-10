extern "C" const unsigned* inline_data(unsigned* observed) {
    const unsigned* address;
    unsigned value;
    asm("jmp 2f\n.p2align 8\n.Lpayload%=:\n.long 0x8357c1e9\n.byte 17,29\n.short 0xb7c3\n.zero 8\n2:\nleaq .Lpayload%=(%%rip),%0\nmovl .Lpayload%=(%%rip),%1"
        : "=&r"(address), "=r"(value));
    *observed = value;
    return address;
}
extern "C" unsigned inline_balign() {
    unsigned value;
    asm("jmp 1f\n.balign 32,0x90\n.byte 0xcc\n1:\nmovl $73,%0" : "=r"(value));
    return value;
}
extern "C" unsigned inline_numeric_address() {
    unsigned value;
    asm("movl 1f(%%rip),%0\n jmp 2f\n.balign 16\n1:\n.long 0x76543210\n2:\n" : "=r"(value));
    return value;
}

extern "C" const unsigned char* inline_target_align() {
    const unsigned char* address;
#if defined(__APPLE__)
    asm("jmp 2f\n.align 6\n1: .byte 0x5a\n2: leaq 1b(%%rip),%0" : "=r"(address));
#else
    asm("jmp 2f\n.align 64\n1: .byte 0x5a\n2: leaq 1b(%%rip),%0" : "=r"(address));
#endif
    return address;
}
