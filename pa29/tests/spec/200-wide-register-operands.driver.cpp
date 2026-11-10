#include <cstdio>
#include <initializer_list>
extern "C" void wide_storage(const unsigned*,const unsigned*,unsigned*);
extern "C" void wide_values(const unsigned*,const unsigned*,unsigned*);
extern "C" void wide_alias_outputs(const unsigned*,unsigned*);
int main() {
    if (!__builtin_cpu_supports("avx2")) {std::puts("AVX2 unavailable; native execution skipped");return 0;}
    unsigned a[10],b[10],out[10];
    for (unsigned i=0;i<10;++i) {a[i]=0xf382a7d9u+i*17;b[i]=0x173931bdu+i*37;out[i]=0xc5a796efu;}
    for (auto operation:{wide_storage,wide_values}) {
        operation(a+1,b+1,out+1);
        for (unsigned i=1;i<9;++i) if (out[i]!=a[i]+b[i]) return 1;
        if (out[0]!=0xc5a796efu || out[9]!=0xc5a796efu) return 2;
    }
    wide_alias_outputs(a+1,out+1);
    for (unsigned i=1;i<9;++i) if (out[i]!=(a[i]<<2)) return 3;
    if (out[0]!=0xc5a796efu || out[9]!=0xc5a796efu) return 4;
    std::puts("wide stored/value inputs, early writes and aliased outputs preserve all bytes");
}
