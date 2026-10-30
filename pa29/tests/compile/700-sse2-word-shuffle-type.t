#include <emmintrin.h>
using result=decltype(_mm_shufflelo_epi16(_mm_setzero_si128(),27));
static_assert(sizeof(result)==16,"128-bit result");
int main(){short out[8];_mm_storeu_si128(reinterpret_cast<__m128i*>(out),_mm_shufflehi_epi16(_mm_set1_epi16(7),27));return out[0]!=7||out[7]!=7;}
