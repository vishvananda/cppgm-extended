#include <emmintrin.h>
#if !__has_builtin(__builtin_ia32_movmskpd) || !__has_builtin(__builtin_ia32_movmskps)
#error floating sign masks must be provided
#endif
static_assert(sizeof(decltype(_mm_movemask_pd(_mm_setzero_pd())))==sizeof(int),"int result");
int main(){return _mm_movemask_pd(_mm_set_pd(-1.0,0.0))!=2||_mm_movemask_ps(_mm_set_ps(-1.0,1.0,-1.0,1.0))!=10;}
