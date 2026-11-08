#if defined(__APPLE__)
#define SYM(name) "_" name
#else
#define SYM(name) name
#endif
asm(".macro make_add name:req,bias=9\n.globl \\name\n\\name:\n"
    "jmp .Lbody\\@\nud2\n.Lbody\\@:\nlea \\bias(%rdi),%rax\n"
    ".if \\bias > 7\nadd $2,%rax\n.elseif \\bias == 3\nadd $4,%rax\n.else\nadd $6,%rax\n.endif\nret\n.endm\n");
asm(".text\nmake_add " SYM("macro_default") "\nmake_add " SYM("macro_keyword") ",bias=3\n"
    ".macro raw_words values:vararg\n.quad \\values\n.endm\n"
    ".data\n.balign 8\n.globl " SYM("macro_words") "\n" SYM("macro_words") ":\n"
    "raw_words 11,13,17\n.irp value,3,5,7\n.quad \\value\n.endr\n"
    ".rept 2 + 1 << 1\n.quad 19\n.endr\n"
    ".globl " SYM("macro_digits") "\n" SYM("macro_digits") ":\n.irpc value,246\n.byte \\value\n.endr\n"
    ".macro message text:req\n.asciz \"\\text\"\n.endm\n"
    ".globl " SYM("macro_message") "\n" SYM("macro_message") ":\nmessage \"hello world\"\n"
    ".macro raw_strings values:vararg\n.ascii \\values\n.endm\n"
    ".globl " SYM("macro_strings") "\n" SYM("macro_strings") ":\nraw_strings \"ab\", \"cd\"\n"
    ".macro default_string text=\"default\"\n.asciz \"\\text\"\n.endm\n"
    ".globl " SYM("macro_default_string") "\n" SYM("macro_default_string") ":\ndefault_string\n"
    ".purgem message\n.macro message\n.byte 31\n.endm\nmessage\n");
