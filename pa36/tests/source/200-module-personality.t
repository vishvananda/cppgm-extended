#if defined(__APPLE__)
#define SYMBOL(name) "_" name
#define RODATA ".section __TEXT,__const\n"
#define PERSONALITY SYMBOL("assembly_personality")
#else
#define SYMBOL(name) name
#define RODATA ".section .rodata\n"
#define PERSONALITY "assembly_personality_cell"
asm(".data\n.balign 8\nassembly_personality_cell:\n.quad assembly_personality\n");
#endif
asm(RODATA ".globl " SYMBOL("assembly_exception_data") "\n" SYMBOL("assembly_exception_data") ":\n.byte 69,103,17,53\n");
asm(".text\n.cfi_sections .eh_frame\n.globl " SYMBOL("assembly_personality_call") "\n"
    SYMBOL("assembly_personality_call") ":\n.cfi_startproc\n"
    ".cfi_personality 0x9b," PERSONALITY "\n.cfi_lsda 0x1b," SYMBOL("assembly_exception_data") "\n"
    "sub $8,%rsp\n.cfi_adjust_cfa_offset 8\ncall *%rdi\nadd $8,%rsp\n.cfi_adjust_cfa_offset -8\nret\n.cfi_endproc\n");
