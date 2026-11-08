#include <xmmintrin.h>
unsigned host_mxcsr(void) { return _mm_getcsr(); }
