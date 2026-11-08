extern "C" __attribute__((naked)) void bad_evex() { asm(".intel_syntax noprefix\nvaddps zmm0,zmm1,dword ptr [rdi]{1to8}\n.att_syntax prefix\nret"); }
