asm(".macro add_pair result:req,value:req\nadd \\value,\\result\n.endm\n"
    ".macro branch_if_negative value:req,target:req\ntest \\value,\\value\njs \\target\n.endm\n"
    ".macro take_address result:req,symbol:req\nlea \\symbol(%rip),\\result\n.endm\n");
extern "C" long macro_storage = 37;
extern "C" long macro_inline_add(long a, long b) {
    asm("add_pair %0,%1" : "+r"(a) : "r"(b) : "cc");
    return a;
}
extern "C" int macro_inline_goto(long value) {
    asm goto("branch_if_negative %0,%l[negative]" : : "r"(value) : "cc" : negative);
    return 3;
negative:
    return 7;
}
extern "C" long* macro_inline_address() {
    long* pointer;
    asm("take_address %0,%c1" : "=r"(pointer) : "i"(&macro_storage));
    return pointer;
}
extern "C" __attribute__((naked)) long macro_raw_add(long, long) {
    asm("mov %rdi,%rax\nadd_pair %rax,%rsi\nret\n");
}
