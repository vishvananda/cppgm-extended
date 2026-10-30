#include <emmintrin.h>
__attribute__((always_inline))inline __m128i wrapper(__m128i value,int selector){return _mm_shuffle_epi32(value,selector);}
__m128i bad(__m128i value,int selector){return wrapper(value,selector);}
