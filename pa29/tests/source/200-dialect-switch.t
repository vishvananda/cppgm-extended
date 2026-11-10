extern "C" long mixed_dialect(long value) {
    long result;
    asm("mov %1,%%rax\n1:\n.intel_syntax noprefix\ncmp rax,0\njge 2f\nneg rax\n"
        ".att_syntax prefix\n2:\nadd $11,%%rax\n.intel_syntax noprefix\nadd rax,3\n"
        ".att_syntax prefix\nadd $7,%%rax\n.att_syntax prefix\n"
        : "=a"(result) : "r"(value) : "cc");
    return result;
}
extern "C" __attribute__((naked)) long mixed_raw(long) {
    asm("mov %rdi,%rax\njmp 2f\n1:\n.att_syntax prefix\nadd $7,%rax\nret\n"
        ".intel_syntax noprefix\n2:\nadd rax,5\njmp 1b\n.att_syntax prefix\n");
}
