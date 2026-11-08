extern "C" __attribute__((naked)) void bad_evex() { asm(".intel_syntax noprefix\nvpslldq zmm0{k1},zmm1,3\n.att_syntax prefix\nret"); }
