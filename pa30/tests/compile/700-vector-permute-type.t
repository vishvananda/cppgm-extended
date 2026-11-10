#include <emmintrin.h>
#if !__has_builtin(__builtin_shufflevector)
#error indexed vector shuffles must be provided
#endif
#if !defined(__clang__) && !__has_builtin(__builtin_shuffle)
#error GNU mask-vector shuffles must be provided
#endif
int main(){unsigned long long a[2]={1,2},b[2]={3,4},out[2];_mm_storeu_si128(reinterpret_cast<__m128i*>(out),_mm_castpd_si128(_mm_move_sd(_mm_castsi128_pd(_mm_loadu_si128(reinterpret_cast<const __m128i*>(a))),_mm_castsi128_pd(_mm_loadu_si128(reinterpret_cast<const __m128i*>(b))))));return out[0]!=3||out[1]!=2;}
