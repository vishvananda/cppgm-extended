// A clamp of a floating value is a scalar minimum or maximum (lane_bound in
// vector.cpp): LowIR makes each bound a vector of one lane, built once before
// the loop and read by every clamp in it. The vector reads only its low lane,
// so it shares the bound's own register or spill: the loop, which also reads
// the bounds as scalars and carries many floating sums, keeps one value for
// each bound instead of two.
static inline double lower(double a, double b) { return b < a ? b : a; }
static inline double upper(double a, double b) { return a < b ? b : a; }

extern "C" __attribute__((noinline)) double crowded(const double* x, int n, double lo, double hi) {
    double s0 = 0.0, s1 = 1.0, s2 = 2.0, s3 = 3.0, s4 = 4.0, s5 = 5.0, s6 = 6.0, s7 = 7.0, s8 = 8.0, s9 = 9.0, s10 = 10.0, s11 = 11.0, s12 = 12.0, s13 = 13.0, s14 = 14.0, s15 = 15.0, s16 = 16.0, s17 = 17.0, s18 = 18.0, s19 = 19.0, s20 = 20.0, s21 = 21.0, s22 = 22.0, s23 = 23.0, s24 = 24.0, s25 = 25.0;
    for (int i = 0; i < n; ++i) {
        const double a = x[4 * i], b = x[4 * i + 1], c = x[4 * i + 2], d = x[4 * i + 3];
        const double p = upper(lower(a, hi), lo);
        const double q = upper(lower(b, hi), lo);
        const double r = upper(lower(c, hi), lo);
        const double t = upper(lower(d, hi), lo);
        s0 = s0 * p;
        s1 += q;
        s2 += r;
        s3 = s3 * t;
        s4 += p;
        s5 += q;
        s6 = s6 * r;
        s7 += t;
        s8 += p;
        s9 = s9 * q;
        s10 += r;
        s11 += t;
        s12 = s12 * p;
        s13 += q;
        s14 += r;
        s15 = s15 * t;
        s16 += p;
        s17 += q;
        s18 = s18 * r;
        s19 += t;
        s20 += p;
        s21 = s21 * q;
        s22 += r;
        s23 += t;
        s24 = s24 * p;
        s25 += q;
        s0 += hi;
        s1 -= lo;
    }
    return s0 + s1 + s2 + s3 + s4 + s5 + s6 + s7 + s8 + s9 + s10 + s11 + s12 + s13 + s14 + s15 + s16 + s17 + s18 + s19 + s20 + s21 + s22 + s23 + s24 + s25;
}
