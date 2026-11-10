#include <cstdio>
extern "C" void upper_vector_source(const unsigned*,const unsigned*,unsigned*);
extern "C" void upper_scalar_source(const double*,const double*,double*);
int main() {
    if (!__builtin_cpu_supports("avx512f") || !__builtin_cpu_supports("avx512vl")) {
        std::puts("AVX-512 VL unavailable; encoding inspection retained");return 0;
    }
    alignas(32) unsigned a[8],b[8],out[8];
    for (unsigned i=0;i<8;++i) {a[i]=0xf8739123u+i*17;b[i]=0x975231fdu-i*37;out[i]=0;}
    upper_vector_source(a,b,out);
    for (unsigned i=0;i<8;++i) if (out[i]!=a[i]+b[i]) return 1;
    double x=19.25,y=31.5,z[3]={71,0,73};
    upper_scalar_source(&x,&y,z+1);
    if (z[0]!=71 || z[1]!=50.75 || z[2]!=73) return 2;
    std::puts("EVEX register class preserves complete vector and scalar values in upper units");
}
