#include <emmintrin.h>
#if !defined(__clang__) && (!__has_builtin(__builtin_ia32_packuswb128) || !__has_builtin(__builtin_ia32_packsswb128) || !__has_builtin(__builtin_ia32_packssdw128))
#error required GNU packed conversion builtins are not provided
#endif
static_assert(sizeof(decltype(_mm_packus_epi16(_mm_setzero_si128(),_mm_setzero_si128())))==16,"packed bytes");
static_assert(sizeof(decltype(_mm_packs_epi32(_mm_setzero_si128(),_mm_setzero_si128())))==16,"packed words");
int main(){unsigned char out[16];_mm_storeu_si128(reinterpret_cast<__m128i*>(out),_mm_packus_epi16(_mm_set1_epi16(256),_mm_set1_epi16(-1)));return out[0]!=255||out[15]!=0;}
