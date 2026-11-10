#include <smmintrin.h>
#if !defined(__clang__) && !__has_builtin(__builtin_ia32_packusdw128)
#error missing packed unsigned word conversion
#endif
static_assert(sizeof(decltype(_mm_packus_epi32(_mm_setzero_si128(), _mm_setzero_si128()))) == 16, "packed words");
