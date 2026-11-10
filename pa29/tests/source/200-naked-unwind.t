extern "C" __attribute__((naked)) void naked_unwind_call(void (*)(), long) {
    asm(
    "push %rbp\n.cfi_def_cfa_offset 16\n.cfi_offset %rbp,-16\n"
    "mov %rsp,%rbp\n.cfi_def_cfa_register %rbp\npush %rbx\n.cfi_rel_offset %rbx,-8\n"
    "sub $8,%rsp\nxor %ebx,%ebx\ntest %rsi,%rsi\nje 1f\n"
    ".cfi_remember_state\nadd $8,%rsp\npop %rbx\n.cfi_restore %rbx\npop %rbp\n"
    ".cfi_restore %rbp\n.cfi_def_cfa %rsp,8\nret\n.cfi_restore_state\n"
    "1:\ncall *%rdi\nadd $8,%rsp\npop %rbx\n.cfi_restore %rbx\npop %rbp\n"
    ".cfi_restore %rbp\n.cfi_def_cfa %rsp,8\nret\n");
}
