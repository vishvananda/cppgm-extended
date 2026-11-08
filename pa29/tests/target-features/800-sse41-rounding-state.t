#include <smmintrin.h>
static unsigned calls;
__attribute__((noinline)) unsigned control(unsigned value) { ++calls; return value; }
__attribute__((noinline)) int check() {
    const unsigned original = _mm_getcsr();
    volatile float source = 1.75f;
    float input[4] = {source, source, source, source};
    const __m128 value = _mm_loadu_ps(input);
    const int expected[4] = {2, 1, 2, 1};
    int result = 0;
    for (unsigned mode = 0; mode != 4; ++mode) {
        const unsigned next = (original & ~0x6000u) | (mode << 13);
        _mm_setcsr(control(next));
        if (_mm_getcsr() != next) result = 1;
        float out[4];
        _mm_storeu_ps(out, _mm_round_ps(value, _MM_FROUND_CUR_DIRECTION | _MM_FROUND_NO_EXC));
        for (unsigned i = 0; i != 4; ++i) if (out[i] != expected[mode]) result = 2;
    }
    _mm_setcsr(original);
    return result || calls != 4 || _mm_getcsr() != original;
}
int main() {
    unsigned a=1,b,c=0,d;
    asm volatile("cpuid" : "+a"(a), "=b"(b), "+c"(c), "=d"(d));
    return (c & (1u << 19)) && check();
}
