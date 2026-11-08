#include <cstdio>
#include <cstring>
#include <initializer_list>
extern "C" void evex_zero(const unsigned*,const unsigned*,unsigned*,unsigned);
extern "C" void evex_merge(const unsigned*,const unsigned*,unsigned*,unsigned);
extern "C" void evex_broadcast(const unsigned*,const unsigned char*,unsigned*);
int main() {
    if (!__builtin_cpu_supports("avx512f")) {std::puts("AVX-512 unavailable; encoding inspection retained");return 0;}
    unsigned a[18],b[18],out[18];
    for (unsigned i=0;i<18;++i) {a[i]=0xf739a123u+i*17;b[i]=0x15397137u+i*37;}
    for (unsigned mask:{0u,0xffffu,0xaaaau,0x5555u,0x8001u}) {
        for (auto& value:out) value=0xda9c7315u;
        evex_zero(a+1,b+1,out+1,mask);
        for (unsigned i=0;i<16;++i) if (out[i+1]!=((mask&(1u<<i))?a[i+1]+b[i+1]:0u)) return 1;
        if (out[0]!=0xda9c7315u || out[17]!=0xda9c7315u) return 2;
        for (auto& value:out) value=0xda9c7315u;
        evex_merge(a+1,b+1,out+1,mask);
        for (unsigned i=0;i<16;++i) if (out[i+1]!=((mask&(1u<<i))?a[i+1]+b[i+1]:0xda9c7315u)) return 3;
        if (out[0]!=0xda9c7315u || out[17]!=0xda9c7315u) return 4;
    }
    unsigned char bias[1024]={};
    for (unsigned i=0;i<5;++i) bias[508+i]=static_cast<unsigned char>(17*(i+1));
    unsigned first=0,second=0;std::memcpy(&first,bias+508,4);std::memcpy(&second,bias+509,4);
    evex_broadcast(a+1,bias,out+1);
    for (unsigned i=1;i<17;++i) if (out[i]!=a[i]+first+second) return 5;
    if (out[0]!=0xda9c7315u || out[17]!=0xda9c7315u) return 6;
    std::puts("EVEX masks, zero/merge results, upper registers, broadcasts and guards passed");
}
