#if defined(__APPLE__)
#define WEAK ".weak_definition _module_weak\n.globl _module_weak,_module_wrap\n"
#define SYMBOL(name) "_" name
#else
#define WEAK ".weak module_weak\n.globl module_wrap\n"
#define SYMBOL(name) name
#endif
extern "C" long module_wrap(long);
asm(".text\n" SYMBOL("module_weak") ":\nlea 1(%rdi),%rax\nret\n"
    SYMBOL("module_wrap") ":\ntest %rdi,%rdi\njs " SYMBOL("module_weak") "\nsub $8,%rsp\ncall " SYMBOL("module_weak") "\nadd $8,%rsp\nret\n");
asm(WEAK);
