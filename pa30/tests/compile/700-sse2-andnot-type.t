#include <emmintrin.h>
#if !defined(__clang__) && !__has_builtin(__builtin_ia32_pandn128)
#error required GNU builtin is not provided
#endif
using result=decltype(_mm_andnot_si128(_mm_setzero_si128(),_mm_setzero_si128()));
static_assert(sizeof(result)==16,"128-bit result");
int main(){unsigned long long out[2];_mm_storeu_si128(reinterpret_cast<__m128i*>(out),_mm_andnot_si128(_mm_setzero_si128(),_mm_set1_epi32(-1)));return out[0]!=~0ULL||out[1]!=~0ULL;}
