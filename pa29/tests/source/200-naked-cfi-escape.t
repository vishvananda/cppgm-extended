extern "C" __attribute__((naked)) void escaped_cfa_expression(void (*)()) {
    asm("sub $8,%rsp\n.cfi_escape 0x0f,2,0x77,16\ncall *%rdi\n"
        "add $8,%rsp\n.cfi_def_cfa %rsp,8\nret\n");
}
extern "C" __attribute__((naked)) void escaped_cfa_bookkeeping(void (*)()) {
    asm("sub $8,%rsp\n.cfi_escape 0x0e,32\n.cfi_adjust_cfa_offset 8\ncall *%rdi\n"
        "add $8,%rsp\n.cfi_adjust_cfa_offset -8\nret\n");
}
