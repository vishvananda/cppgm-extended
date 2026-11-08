#if defined(__APPLE__)
#define ASM_SYMBOL(name) "_" name
#define ASM_TYPE(name)
#define ASM_SIZE(name)
#else
#define ASM_SYMBOL(name) name
#define ASM_TYPE(name) ".type " name ",@function\n"
#define ASM_SIZE(name) ".size " name ",.-" name "\n"
#endif
extern "C" long module_add(long);
extern "C" long module_callback(long);
extern "C" long module_call(long);
extern "C" long module_data[3];
extern "C" long* module_pointer;
asm(".text\n.globl " ASM_SYMBOL("module_add") "\n" ASM_TYPE("module_add")
    ASM_SYMBOL("module_add") ":\nlea 7(%rdi),%rax\nret\n" ASM_SIZE("module_add"));
asm(".data\n.balign 16\n.globl " ASM_SYMBOL("module_data") "\n"
    ASM_SYMBOL("module_data") ":\n.quad 11,13,17\n"
    ".globl " ASM_SYMBOL("module_pointer") "\n" ASM_SYMBOL("module_pointer") ":\n.quad " ASM_SYMBOL("module_data") "+8\n");
asm(".text\n.globl " ASM_SYMBOL("module_call") "\n" ASM_SYMBOL("module_call") ":\n"
    "sub $8,%rsp\ncall " ASM_SYMBOL("module_callback") "\nadd $8,%rsp\nret\n");
extern "C" long module_read(long value) { return module_add(value) + module_data[1]; }
