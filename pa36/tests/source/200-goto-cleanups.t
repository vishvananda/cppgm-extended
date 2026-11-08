template<int Bound> static inline int threshold(int value) {
    asm goto("cmp %1, %0; jge %l[high]" : : "r"(value), "i"(Bound) : "cc" : high);
    return 0;
high:
    return 1;
}
extern "C" int goto_templates(int value) {
    return threshold<3>(value) + 2 * threshold<17>(value);
}
extern "C" int goto_all_gprs() {
    asm goto("jmp %l[done]" : : :
        "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9",
        "r10", "r11", "r12", "r13", "r14", "r15" : done);
    return -1;
done:
    return 37;
}
