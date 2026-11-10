#include <emmintrin.h>
__m128i bad(int value){return _mm_shufflehi_epi16(value,27);}
