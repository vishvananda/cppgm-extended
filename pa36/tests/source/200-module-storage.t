#if defined(__APPLE__)
#define SYMBOL(name) "_" name
#define RODATA ".section __TEXT,__const\n"
#define TYPE(name)
#define SIZE(name)
#else
#define SYMBOL(name) name
#define RODATA ".section \".rodata.module_values\",\"a\",@progbits\n"
#define TYPE(name) ".type " name ",@object\n"
#define SIZE(name) ".size " name ",.-" name "\n"
#endif
extern "C" const unsigned char module_message[];
extern "C" const unsigned long module_numbers[];
extern "C" const unsigned long module_alias[];
extern "C" unsigned char module_zeroes[];
asm(RODATA ".globl " SYMBOL("module_message") "\n" SYMBOL("module_message")
    " : .ascii \"a;#:\"\n.byte 0,0377,-1\n.asciz \"b\\n\"\n"
    ".p2align 5\n.globl " SYMBOL("module_numbers") "\n" TYPE("module_numbers")
    SYMBOL("module_numbers") ":\n.quad 11,0x123456789abcdef0,-1\n" SIZE("module_numbers")
    ".globl " SYMBOL("module_alias") "\n.set " SYMBOL("module_alias") "," SYMBOL("module_numbers") "+8\n");
asm(".bss\n.p2align 5\n.globl " SYMBOL("module_zeroes") "\n"
    SYMBOL("module_zeroes") ":\n.zero 37\n");
#if !defined(__APPLE__)
asm(".data\n.pushsection .rodata.module_stack,\"a\",@progbits\n"
    ".byte 3\n.popsection\n.globl module_after_stack\nmodule_after_stack:\n.long 0x12345678\n");
#endif
