#include <emmintrin.h>
__m128i bad(__m128i value){return _mm_shufflehi_epi16(value);}
