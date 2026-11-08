extern "C" __attribute__((naked)) long naked_sum(long, long) {
    asm("lea (%rdi,%rsi,2), %rax; ret");
}
extern "C" __attribute__((naked)) long naked_local(long) {
    asm("push %rbx; mov %rdi, %rbx; call 1f; pop %rbx; ret");
    asm("1: lea 3(%rbx,%rbx,2), %rax; ret");
}
extern "C" __attribute__((naked)) long naked_callback(long (*)(long), long) {
    asm("mov %rdi, %rax; mov %rsi, %rdi; sub $8, %rsp; call *%rax; add $8, %rsp; add $5, %rax; ret");
}
extern "C" __attribute__((naked)) long naked_tail(long (*)(long), long) {
    asm("mov %rdi, %rax; mov %rsi, %rdi; jmp *%rax");
}
extern "C" long naked_same_unit(long a, long b) {
    return naked_sum(a, b) + naked_sum(b, a) + 11;
}
