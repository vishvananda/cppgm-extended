extern "C" __attribute__((naked)) void bad_evex() { asm(".intel_syntax noprefix\nvaddps zmm0{z},zmm1,zmm2\n.att_syntax prefix\nret"); }
