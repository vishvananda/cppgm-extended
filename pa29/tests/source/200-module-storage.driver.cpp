#include <cstdio>
extern "C" const unsigned char module_message[];
extern "C" const unsigned long module_numbers[];
extern "C" const unsigned long module_alias[];
extern "C" unsigned char module_zeroes[];
#if !defined(__APPLE__)
extern "C" unsigned module_after_stack;
#endif
int main() {
    const unsigned char expected[] = {'a',';','#',':',0,255,255,'b','\n',0};
    for(unsigned i=0;i<sizeof expected;++i) if(module_message[i]!=expected[i])return 1;
    if(module_numbers[0]!=11 || module_numbers[1]!=0x123456789abcdef0ul || module_numbers[2]!=~0ul)return 2;
    if(module_alias!=module_numbers+1)return 3;
    if((reinterpret_cast<unsigned long>(module_numbers)&31)!=0)return 4;
    if((reinterpret_cast<unsigned long>(module_zeroes)&31)!=0)return 5;
    for(unsigned i=0;i<37;++i)if(module_zeroes[i]!=0)return 6;
    module_zeroes[36]=71;
    if(module_zeroes[36]!=71)return 7;
#if !defined(__APPLE__)
    if(module_after_stack!=0x12345678u)return 8;
#endif
    std::puts("module section state, strings, aliases, alignment and bss passed");
}
