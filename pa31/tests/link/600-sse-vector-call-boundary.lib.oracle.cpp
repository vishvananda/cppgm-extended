#define P(n) ba_##n
#include "600-sse-vector-call-boundary.shared.h"
void ps(__m128 v);
void pd(__m128d v);
void pi(__m128i v);
double ba_var(int n, ...){ va_list ap; va_start(ap,n); double t=0; for(int k=0;k<n;++k){ __m128d v=va_arg(ap,__m128d); double u[2]; _mm_storeu_pd(u,v); t+=u[0]+2*u[1]; t+=va_arg(ap,int); } va_end(ap); return t; }
void host_calls()
{
	__m128 a = _mm_set_ps(4, 3, 2, 1), b = _mm_set_ps(.5f, .25f, .125f, 1);
	ps(ab_ret(a, b));
	__m128d d[10];
	for (int i = 0; i < 10; ++i)
		d[i] = _mm_set_pd(i * 3, i + 2);
	pd(ab_many(d[0], d[1], d[2], d[3], d[4], d[5], d[6], d[7], d[8], d[9], 1.5, 4));
	pi(ab_mix(2, _mm_set1_epi32(6), 3, 4.0, _mm_set_epi32(5, 6, 7, 8)));
	Wrap w;
	w.v = b;
	ps(ab_wrap(w).v);
	Pair p;
	p.a = d[2];
	p.b = d[3];
	Pair q = ab_pair(p, d[4]);
	pd(q.a);
	pd(q.b);
	Mixed m;
	m.i = 7;
	m.d = 1.25;
	m.v = _mm_set_epi32(4, 3, 2, 1);
	Mixed n = ab_mixed(m);
	printf("%d %g\n", n.i, n.d);
	pi(n.v);
}
