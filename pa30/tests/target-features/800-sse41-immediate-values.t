#include <smmintrin.h>
template<int M> bool masks() {
    float a[4] = {2, 3, 5, 7}, b[4] = {11, 13, 17, 19}, got[4];
    _mm_storeu_ps(got, _mm_dp_ps(_mm_loadu_ps(a), _mm_loadu_ps(b), M));
    float sum = 0;
    for (unsigned i = 0; i != 4; ++i) if (M & (16u << i)) sum += a[i]*b[i];
    for (unsigned i = 0; i != 4; ++i) if (got[i] != ((M & (1u << i)) ? sum : 0)) return false;
    _mm_storeu_ps(got, _mm_insert_ps(_mm_loadu_ps(a), _mm_loadu_ps(b), M));
    for (unsigned i = 0; i != 4; ++i) {
        float expected = i == ((M >> 4) & 3) ? b[(M >> 6) & 3] : a[i];
        if (M & (1u << i)) expected = 0;
        if (got[i] != expected) return false;
    }
    double x[2] = {2, 3}, y[2] = {11, 13}, d[2];
    _mm_storeu_pd(d, _mm_dp_pd(_mm_loadu_pd(x), _mm_loadu_pd(y), M));
    double ds = ((M & 16) ? 22 : 0) + ((M & 32) ? 39 : 0);
    for (unsigned i = 0; i != 2; ++i) if (d[i] != ((M & (1u << i)) ? ds : 0)) return false;
    return masks<M-1>();
}
template<> bool masks<0>() { return true; }
template<int M> bool rounds() {
    float a[4] = {1.25f, -1.75f, 3.5f, -2.5f}, b[4] = {8, 9, 10, 11}, out[4];
    float expected[4];
    for (unsigned i = 0; i != 4; ++i) {
        int n = static_cast<int>(a[i]);
        if ((M & 7) == 1 && a[i] < n) --n;
        if ((M & 7) == 2 && a[i] > n) ++n;
        if ((M & 7) == 0 || (M & 4)) {
            float delta = a[i] - n;
            if (delta > 0.5f || (delta == 0.5f && (n & 1))) ++n;
            if (delta < -0.5f || (delta == -0.5f && (n & 1))) --n;
        }
        expected[i] = n;
    }
    _mm_storeu_ps(out, _mm_round_ps(_mm_loadu_ps(a), M));
    for (unsigned i = 0; i != 4; ++i) if (out[i] != expected[i]) return false;
    _mm_storeu_ps(out, _mm_round_ss(_mm_loadu_ps(b), _mm_loadu_ps(a), M));
    if (out[0] != expected[0] || out[1] != b[1] || out[2] != b[2] || out[3] != b[3]) return false;
    double d[2] = {a[0], a[1]}, upper[2] = {13, 17}, result[2];
    _mm_storeu_pd(result, _mm_round_pd(_mm_loadu_pd(d), M));
    if (result[0] != expected[0] || result[1] != expected[1]) return false;
    _mm_storeu_pd(result, _mm_round_sd(_mm_loadu_pd(upper), _mm_loadu_pd(d), M));
    if (result[0] != expected[0] || result[1] != upper[1]) return false;
    return rounds<M-1>();
}
template<> bool rounds<0>() { return true; }
int main() {
    unsigned a=1,b,c=0,d;
    asm volatile("cpuid" : "+a"(a), "=b"(b), "+c"(c), "=d"(d));
    if (!(c & (1u << 19))) return 0;
    return !masks<255>() || !rounds<15>();
}
