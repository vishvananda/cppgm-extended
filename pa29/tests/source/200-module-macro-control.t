#if defined(__APPLE__)
#define SYM(name) "_" name
#else
#define SYM(name) name
#endif
asm(".data\n.globl " SYM("macro_control") "\n" SYM("macro_control") ":\n"
    ".macro outer\n.macro inner value\n.byte \\value\n.endm\n.endm\nouter\ninner 7\n"
    ".macro stop\n.byte 1\n.rept 3\n.byte 2\n.exitm\n.byte 3\n.endr\n.byte 4\n.endm\nstop\n.byte 5\n"
    ".macro old\n.purgem old\n.byte 6\n.endm\nold\n"
    ".macro defaults first = 11,second = 13\n.byte \\first,\\second\n.endm\n"
    "defaults ,17\ndefaults second = 19, first = 23\n"
    ".if 0\ninvalid skipped instruction\n.elseif 3 == 1 + 2\n.byte 31\n.else\n.byte 0\n.endif\n"
    ".ifc red,red\n.byte 37\n.endif\n.ifnc alpha,beta\n.byte 41\n.endif\n"
    ".ifeqs \"north west\",\"north west\"\n.byte 43\n.endif\n"
    ".macro blank value\n.ifb \\value\n.byte 47\n.else\n.byte 0\n.endif\n.endm\nblank\n"
    ".macro recursive left\n.byte 53\n.if \\left\nrecursive \"(\\left-1)\"\n.endif\n.endm\nrecursive 3\n");
