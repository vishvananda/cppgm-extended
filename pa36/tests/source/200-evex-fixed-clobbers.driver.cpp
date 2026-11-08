#include <cstdio>
extern "C" void evex_source_words(const unsigned*,const unsigned*,unsigned*,unsigned);
int main() {
    if (!__builtin_cpu_supports("avx512f")) {std::puts("AVX-512 unavailable; encoding inspection retained");return 0;}
    unsigned a[18],b[18],out[18];
    for (unsigned i=0;i<18;++i) {a[i]=0xf8739123u+i*17;b[i]=0x975231fdu-i*37;out[i]=0xcafef00du;}
    evex_source_words(a+1,b+1,out+1,0xa5c3);
    for (unsigned i=0;i<16;++i) if (out[i+1]!=((0xa5c3u&(1u<<i))?a[i+1]+b[i+1]:0u)) return 1;
    if (out[0]!=0xcafef00du || out[17]!=0xcafef00du) return 2;
    std::puts("fixed upper ZMM/opmask clobbers retain masked values and guard words");
}
