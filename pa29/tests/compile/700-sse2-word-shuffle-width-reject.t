#include <emmintrin.h>
using small=short __attribute__((vector_size(8)));
__m128i bad(small value){return _mm_shufflelo_epi16(value,27);}
