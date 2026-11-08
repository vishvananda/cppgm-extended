extern "C" __attribute__((naked)) void bad_evex() { asm(".intel_syntax noprefix\nadd rax{k1},rcx\n.att_syntax prefix\nret"); }
