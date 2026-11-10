#if defined(__APPLE__)
#define SYMBOL(name) "_" name
#else
#define SYMBOL(name) name
#endif
extern "C" void assembly_simple_unwind_call(void (*)(), long);
asm(".text\n.globl " SYMBOL("assembly_simple_unwind_call") "\n" SYMBOL("assembly_simple_unwind_call") ":\n"
    ".cfi_startproc simple\n.cfi_def_cfa %rsp,8\n.cfi_offset %rip,-8\npush %rbp\n.cfi_def_cfa_offset 16\n.cfi_offset %rbp,-16\n"
    "mov %rsp,%rbp\n.cfi_def_cfa_register %rbp\npush %rbx\n.cfi_rel_offset %rbx,-8\n"
    "sub $8,%rsp\nxor %ebx,%ebx\ntest %rsi,%rsi\nje 1f\n"
    ".cfi_remember_state\nadd $8,%rsp\npop %rbx\n.cfi_restore %rbx\npop %rbp\n"
    ".cfi_restore %rbp\n.cfi_def_cfa %rsp,8\nret\n.cfi_restore_state\n"
    "1:\ncall *%rdi\nadd $8,%rsp\npop %rbx\n.cfi_restore %rbx\npop %rbp\n"
    ".cfi_restore %rbp\n.cfi_def_cfa %rsp,8\nret\n.cfi_endproc\n");
