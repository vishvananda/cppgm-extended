#include <cstdint>
#include <cstdio>
#include <cstring>
using Vector = float __attribute__((vector_size(16)));
extern "C" unsigned long live(unsigned long,unsigned long,unsigned long,unsigned long,
    unsigned long,unsigned long,unsigned long,unsigned long,Vector,Vector*);
int main() {
    const uint32_t bits[4] = {0x11223344,0x55667788,0x99aabbcc,0xddeeff00};
    Vector input;
    std::memcpy(&input,bits,16);
    for (unsigned long seed = 1; seed < 50; ++seed) {
        const unsigned long a=seed,b=seed+1,c=seed+2,d=seed+3,e=seed+4,f=seed+5,g=seed+6,h=seed+7;
        register unsigned long preserved asm("r12") = 0x98abcdef12345678UL;
        asm volatile("" : "+r"(preserved));
        Vector output;
        const unsigned long actual = live(a,b,c,d,e,f,g,h,input,&output);
        asm volatile("" : "+r"(preserved));
        const unsigned long expected = (a*3+b*5)^(c*7+d*11)^(e*13+f*17)^(g*19+h*23);
        if (actual!=expected || preserved!=0x98abcdef12345678UL || std::memcmp(&output,bits,16)) return 1;
    }
    std::puts("clobbers: live inputs, full vector and callee-save passed");
}
