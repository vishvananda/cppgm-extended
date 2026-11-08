#include <cstdio>
extern "C" void vex_source_words(const unsigned*,const unsigned*,unsigned*);
int main() {
    if (!__builtin_cpu_supports("avx2")) {
        std::puts("AVX2 unavailable; native execution skipped");return 0;
    }
    unsigned a[10],b[10],out[10];
    for (unsigned i=0;i<10;++i) {a[i]=0xf8739123u+i*17;b[i]=0x975231fdu-i*37;out[i]=0xcafef00du;}
    vex_source_words(a+1,b+1,out+1);
    for (unsigned i=1;i<9;++i) if (out[i]!=a[i]+b[i]) return 1;
    if (out[0]!=0xcafef00du || out[9]!=0xcafef00du) return 2;
    std::puts("fixed YMM clobbers preserve memory values and guard words");
}
