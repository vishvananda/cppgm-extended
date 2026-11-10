#include <cstdio>
#include <cstdint>
extern "C" void vex_words(const std::uint32_t*,const std::uint32_t*,std::uint32_t*);
extern "C" void vex_lanes(const std::uint32_t*,std::uint32_t*);
extern "C" void vex_bytes(const signed char*,const unsigned char*,unsigned char*);
extern "C" void vex_scalar(const double*,double*);
extern "C" void vex_fused(const float*,const float*,float*);
int main() {
    if (!__builtin_cpu_supports("avx2")) {
        std::puts("AVX2 unavailable; native execution skipped"); return 0;
    }
    std::uint32_t left[10],right[10],output[16];
    for (unsigned i=0;i<10;++i) {left[i]=0xf1234567u+i*19297;right[i]=0x13e2d4c7u+i*37;}
    for (auto& value:output) value=0xd5a743b1u;
    vex_words(left+1,right+1,output+1);
    for (unsigned i=0;i<8;++i)
        if (output[i+1]!=((((left[i+1]+right[i+1])<<3)>>1)^right[i+1])) return 1;
    if (output[0]!=0xd5a743b1u || output[9]!=0xd5a743b1u) return 2;
    vex_lanes(left+1,output+1);
    for (unsigned i=0;i<4;++i)
        if (output[i+1]!=left[8-i] || output[i+5]!=left[8-i] || output[i+9]!=left[i+1]) return 3;
    if (output[0]!=0xd5a743b1u || output[13]!=0xd5a743b1u) return 4;
    signed char bytes[32];unsigned char byte_output[34],bias=19;
    for (unsigned i=0;i<32;++i) bytes[i]=static_cast<signed char>(i*17);
    for (auto& value:byte_output) value=0xc7;
    vex_bytes(bytes,&bias,byte_output+1);
    for (unsigned i=0;i<32;++i) {
        const int value=bytes[i];
        if (byte_output[i+1]!=static_cast<unsigned char>((value<0?-value:value)+bias)) return 5;
    }
    if (byte_output[0]!=0xc7 || byte_output[33]!=0xc7) return 6;
    const double values[]={3,7,11,13,17};double scalar[6]={29,0,0,0,0,31};
    vex_scalar(values,scalar+1);
    if (scalar[0]!=29 || scalar[1]!=11 || scalar[2]!=7 || scalar[3]!=20 || scalar[4]!=7 || scalar[5]!=31) return 7;
    if (__builtin_cpu_supports("fma")) {
        float a[8],b[8],c[8];
        for (unsigned i=0;i<8;++i) {a[i]=float(i+1);b[i]=float(i+3);c[i]=float(i+7);}
        vex_fused(a,b,c);
        for (unsigned i=0;i<8;++i) if (c[i]!=a[i]*b[i]+float(i+7)) return 8;
    }
    std::puts("VEX words, lane shuffles, broadcasts, scalar merges, guards and fused arithmetic passed");
}
