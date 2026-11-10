#include <cpuid.h>
void cpuid_oracle(unsigned leaf,unsigned sub,unsigned* out){__cpuid_count(leaf,sub,out[0],out[1],out[2],out[3]);}
int cpuid_preserve(int(*f)(void)){register unsigned long saved __asm__("rbx")=0x123456789abcdef0UL;__asm__ volatile("":"+r"(saved));int value=f();__asm__ volatile("":"+r"(saved));return value||saved!=0x123456789abcdef0UL;}
