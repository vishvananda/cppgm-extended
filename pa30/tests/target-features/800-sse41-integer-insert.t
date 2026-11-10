#include <smmintrin.h>
template<int N> bool bytes() {
    unsigned char input[16], out[16];
    for (int i=0;i<16;++i) input[i] = i;
    const __m128i v = _mm_loadu_si128(reinterpret_cast<const __m128i*>(input));
    _mm_storeu_si128(reinterpret_cast<__m128i*>(out), _mm_insert_epi8(v, -1, N));
    for (int i=0;i<16;++i) if (out[i] != (i == N ? 255 : input[i])) return false;
    return bytes<N-1>();
}
template<> bool bytes<0>() { return true; }
int main() {
    unsigned a=1,b,c=0,d;
    asm volatile("cpuid" : "+a"(a), "=b"(b), "+c"(c), "=d"(d));
    if (!(c & (1u << 19))) return 0;
    int words[4] = {1,2,3,4}, out[4];
    const __m128i v = _mm_loadu_si128(reinterpret_cast<const __m128i*>(words));
    _mm_storeu_si128(reinterpret_cast<__m128i*>(out), _mm_insert_epi32(v, -1234567, 2));
    if (out[0]!=1 || out[1]!=2 || out[2]!=-1234567 || out[3]!=4) return 1;
    long long q[2] = {11,13}, r[2];
    _mm_storeu_si128(reinterpret_cast<__m128i*>(r), _mm_insert_epi64(_mm_loadu_si128(reinterpret_cast<const __m128i*>(q)), -0x123456789abcdefLL, 1));
    return r[0]!=11 || r[1]!=-0x123456789abcdefLL || !bytes<15>();
}
