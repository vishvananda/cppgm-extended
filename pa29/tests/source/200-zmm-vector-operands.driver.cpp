#include <cstdio>
extern "C" void zmm_source(const unsigned*,const unsigned*,unsigned*);
extern "C" void zmm_source_update(unsigned*,const unsigned*);
extern "C" unsigned zmm_source_aligned(const unsigned*,const unsigned*,unsigned*);
int main() {
    if (!__builtin_cpu_supports("avx512f")) {std::puts("AVX-512 unavailable; encoding inspection retained");return 0;}
    alignas(64) unsigned a[16],b[16],out[16];
    for (unsigned i=0;i<16;++i) {a[i]=0xf329db73u+i*19;b[i]=0x35216779u+i*37;out[i]=0;}
    zmm_source(a,b,out);
    for (unsigned i=0;i<16;++i) if (out[i]!=a[i]+b[i]) return 1;
    zmm_source_update(out,b);
    for (unsigned i=0;i<16;++i) if (out[i]!=a[i]+2*b[i]) return 2;
    if (zmm_source_aligned(a,b,out)!=a[3]+b[5]) return 3;
    for (unsigned i=0;i<16;++i) if (out[i]!=a[i]+b[i]) return 4;
    std::puts("GNU 512-bit vector operands retain complete values, ties and aligned local storage");
}
