#include <emmintrin.h>
__m128i bad(__m128i value,int selector){return _mm_shufflelo_epi16(value,selector);}
