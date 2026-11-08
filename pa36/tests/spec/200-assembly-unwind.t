assembly @unwind_body att readwrite 17 0 1 0 {
 text "push %rbp\n.cfi_def_cfa_offset 16\n.cfi_offset %rbp,-16\nmov %rsp,%rbp\n.cfi_def_cfa_register %rbp\npush %rbx\n.cfi_rel_offset %rbx,-8\nsub $8,%rsp\nxor %ebx,%ebx\ntest %rsi,%rsi\nje 1f\n.cfi_remember_state\nadd $8,%rsp\npop %rbx\n.cfi_restore %rbx\npop %rbp\n.cfi_restore %rbp\n.cfi_def_cfa %rsp,8\nret\n.cfi_restore_state\n1:\ncall *%rdi\nadd $8,%rsp\npop %rbx\n.cfi_restore %rbx\npop %rbp\n.cfi_restore %rbp\n.cfi_def_cfa %rsp,8\nret\n"
}
declare function @lowir_unwind_call(%callback : ptr, %early : i64) -> void
assembly_function @lowir_unwind_call initial {
 block ^entry:
  asm @unwind_body()
  unreachable
}
