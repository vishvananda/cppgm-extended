#include <emmintrin.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
struct Wrap { __m128 v; };
struct Pair { __m128d a; __m128d b; };
struct Mixed { int i; __m128i v; double d; };
__m128 ab_ret(__m128 a, __m128 b);
__m128d ab_many(__m128d a0, __m128d a1, __m128d a2, __m128d a3, __m128d a4, __m128d a5, __m128d a6, __m128d a7, __m128d a8, __m128d a9, double d, int i);
__m128i ab_mix(int x, __m128i v, long y, double z, __m128i w);
Wrap ab_wrap(Wrap w);
Pair ab_pair(Pair p, __m128d x);
Mixed ab_mixed(Mixed m);
double ab_var(int n, ...);
int ab_addr(__m128* p);
__m128 ba_ret(__m128 a, __m128 b);
__m128d ba_many(__m128d a0, __m128d a1, __m128d a2, __m128d a3, __m128d a4, __m128d a5, __m128d a6, __m128d a7, __m128d a8, __m128d a9, double d, int i);
__m128i ba_mix(int x, __m128i v, long y, double z, __m128i w);
Wrap ba_wrap(Wrap w);
Pair ba_pair(Pair p, __m128d x);
Mixed ba_mixed(Mixed m);
double ba_var(int n, ...);
void host_calls();
#ifdef P
__m128 P(ret)(__m128 a, __m128 b){ return _mm_sub_ps(a,b); }
__m128d P(many)(__m128d a0, __m128d a1, __m128d a2, __m128d a3, __m128d a4, __m128d a5, __m128d a6, __m128d a7, __m128d a8, __m128d a9, double d, int i){
 __m128d s=_mm_add_pd(a0,_mm_mul_pd(a1,_mm_set1_pd(2)));
 s=_mm_add_pd(s,_mm_mul_pd(a2,_mm_set1_pd(3))); s=_mm_add_pd(s,_mm_mul_pd(a3,_mm_set1_pd(4)));
 s=_mm_add_pd(s,_mm_mul_pd(a4,_mm_set1_pd(5))); s=_mm_add_pd(s,_mm_mul_pd(a5,_mm_set1_pd(6)));
 s=_mm_add_pd(s,_mm_mul_pd(a6,_mm_set1_pd(7))); s=_mm_add_pd(s,_mm_mul_pd(a7,_mm_set1_pd(8)));
 s=_mm_add_pd(s,_mm_mul_pd(a8,_mm_set1_pd(9))); s=_mm_add_pd(s,_mm_mul_pd(a9,_mm_set1_pd(10)));
 return _mm_add_pd(s,_mm_set_pd(d,(double)i)); }
__m128i P(mix)(int x, __m128i v, long y, double z, __m128i w){ return _mm_add_epi32(_mm_add_epi32(v,w),_mm_set_epi32(x,(int)y,(int)z,1)); }
Wrap P(wrap)(Wrap w){ Wrap r; r.v=_mm_add_ps(w.v,w.v); return r; }
Pair P(pair)(Pair p, __m128d x){ Pair r; r.a=_mm_add_pd(p.a,x); r.b=_mm_sub_pd(p.b,x); return r; }
Mixed P(mixed)(Mixed m){ m.i+=1; m.d+=1; m.v=_mm_add_epi32(m.v,_mm_set1_epi32(3)); return m; }
#endif
