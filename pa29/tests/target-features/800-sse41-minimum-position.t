#include <smmintrin.h>
__attribute__((noinline)) int check() {
    for (unsigned position=0;position<9;++position) {
        unsigned short input[8], out[8];
        for (unsigned i=0;i<8;++i) input[i] = position == 8 ? 65535 : 50000;
        if (position<8) { input[position]=17; if (position<7) input[position+1]=17; }
        const __m128i value = _mm_loadu_si128(reinterpret_cast<const __m128i*>(input));
        _mm_storeu_si128(reinterpret_cast<__m128i*>(out), _mm_minpos_epu16(value));
        if (out[0]!=(position==8?65535:17) || out[1]!=(position==8?0:position)) return 1;
        for (unsigned i=2;i<8;++i) if (out[i]) return 2;
    }
    return 0;
}
int main() {
    unsigned a=1,b,c=0,d;
    asm volatile("cpuid" : "+a"(a), "=b"(b), "+c"(c), "=d"(d));
    return (c & (1u << 19)) && check();
}
