asm(".macro module_label\n.data\n.Lraw_unique\\@:\n.byte 1\n.text\n.endm\nmodule_label\n.purgem module_label\n");
extern "C" __attribute__((naked)) long raw_macro_first(long) {
    asm(".macro first_body\njmp .Lraw_unique\\@\nud2\n.Lraw_unique\\@:\nlea 11(%rdi),%rax\nret\n.endm\nfirst_body\n.purgem first_body\n");
}
extern "C" __attribute__((naked)) long raw_macro_second(long) {
    asm(".macro second_body\njmp .Lraw_unique\\@\nud2\n.Lraw_unique\\@:\nlea 23(%rdi),%rax\nret\n.endm\nsecond_body\n.purgem second_body\n");
}
