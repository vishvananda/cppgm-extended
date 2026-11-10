#include <cstdio>
extern "C" void wide_source(const unsigned*,const unsigned*,unsigned*);
extern "C" void wide_source_update(unsigned*,const unsigned*);
extern "C" unsigned wide_source_aligned(const unsigned*,const unsigned*,unsigned*);
int main() {
    if (!__builtin_cpu_supports("avx2")) {std::puts("AVX2 unavailable; native execution skipped");return 0;}
    alignas(64) unsigned a[8],b[8],out[8];
    for (unsigned i=0;i<8;++i) {a[i]=0xf329db73u+i*19;b[i]=0x35216779u+i*37;out[i]=0;}
    wide_source(a,b,out);
    for (unsigned i=0;i<8;++i) if (out[i]!=a[i]+b[i]) return 1;
    wide_source_update(out,b);
    for (unsigned i=0;i<8;++i) if (out[i]!=a[i]+2*b[i]) return 2;
    if (wide_source_aligned(a,b,out)!=a[3]+b[5]) return 3;
    for (unsigned i=0;i<8;++i) if (out[i]!=a[i]+b[i]) return 4;
    std::puts("GNU wide vector operands retain complete values, ties and aligned local storage");
}
