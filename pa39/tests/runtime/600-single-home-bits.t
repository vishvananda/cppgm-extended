// A loop of 34 single-precision sums, some of them clamps, leaves more of them
// than the SIMD registers hold: some live in general registers and some in
// four-byte homes. A single read from its home into a general register must
// leave the register's high half clear, as its bits are then zero-extended
// with no further instruction. (Found by a generator; kept as generated, with
// junk left in the stack first.)
typedef unsigned long U;
static U bitsf(float v) {
    unsigned u;
    if (v != v)
        return 0x7fc0;
    __builtin_memcpy(&u, &v, 4);
    return u;
}
static U mixh(U h, U v) { return (h ^ v) * 0x100000001b3UL + (h >> 29); }
volatile double vzero = 0.0;
extern "C" __attribute__((noinline)) double ext(double x) { return x * 0.5; }
static __attribute__((noinline)) U f1(const float* x, int n, float lo, float hi) {
    float s0 = (float)0, s1 = (float)1, s2 = (float)2, s3 = (float)3, s4 = (float)4, s5 = (float)5, s6 = (float)6, s7 = (float)7, s8 = (float)8, s9 = (float)9, s10 = (float)10, s11 = (float)11, s12 = (float)12, s13 = (float)13, s14 = (float)14, s15 = (float)15, s16 = (float)16, s17 = (float)17, s18 = (float)18, s19 = (float)19, s20 = (float)20, s21 = (float)21, s22 = (float)22, s23 = (float)23, s24 = (float)24, s25 = (float)25, s26 = (float)26, s27 = (float)27, s28 = (float)28, s29 = (float)29, s30 = (float)30, s31 = (float)31, s32 = (float)32, s33 = (float)33;
    const float c0 = (float)0.0;
    for (int i = 0; i < n; ++i) {
        const float a = x[i], b = x[(i + 1) % n], c = x[(i + 3) % n];
        const float m0 = (b < hi ? hi : b);
        const float m1 = (hi < (b < s6 ? s6 : b) ? hi : (b < s6 ? s6 : b));
        const float m2 = ((s25 > (float)3.25 ? (float)3.25 : s25) < lo ? lo : (s25 > (float)3.25 ? (float)3.25 : s25));
        const float m3 = (c0 < (c < lo ? lo : c) ? c0 : (c < lo ? lo : c));
        const float m4 = (lo > (hi > a ? hi : a) ? lo : (hi > a ? hi : a));
        const float m5 = (c0 > b ? c0 : b);
        const float m6 = (hi < (lo > b ? lo : b) ? hi : (lo > b ? lo : b));
        const float m7 = ((hi > c ? hi : c) < hi ? hi : (hi > c ? hi : c));
        s0 += m7;
        s1 += m1;
        s2 = s2 * m5;
        s3 += m1;
        s4 += m0;
        s5 = s5 * m2;
        s6 += m5;
        s7 = s7 * m6;
        s8 += m3;
        s9 += m0;
        s10 += m3;
        s11 = s11 * m3;
        s12 += m0;
        s13 = s13 * m0;
        s14 = s14 * m3;
        s15 += m4;
        s16 += m3;
        s17 += m7;
        s18 += m7;
        s19 = s19 * m2;
        s20 = s20 * m0;
        s21 += m5;
        s22 = s22 * m1;
        s23 += m2;
        s24 += m1;
        s25 += m3;
        s26 = s26 * m6;
        s27 += m1;
        s28 += m0;
        s29 += m2;
        s30 += m2;
        s31 = s31 * m6;
        s32 = s32 * m5;
        s33 += m6;
        s0 = (float)ext((double)s0);
        s1 -= lo;
        s0 += hi;
    }
    U h = 0;
    h = mixh(h, bitsf(s0));
    h = mixh(h, bitsf(s1));
    h = mixh(h, bitsf(s2));
    h = mixh(h, bitsf(s3));
    h = mixh(h, bitsf(s4));
    h = mixh(h, bitsf(s5));
    h = mixh(h, bitsf(s6));
    h = mixh(h, bitsf(s7));
    h = mixh(h, bitsf(s8));
    h = mixh(h, bitsf(s9));
    h = mixh(h, bitsf(s10));
    h = mixh(h, bitsf(s11));
    h = mixh(h, bitsf(s12));
    h = mixh(h, bitsf(s13));
    h = mixh(h, bitsf(s14));
    h = mixh(h, bitsf(s15));
    h = mixh(h, bitsf(s16));
    h = mixh(h, bitsf(s17));
    h = mixh(h, bitsf(s18));
    h = mixh(h, bitsf(s19));
    h = mixh(h, bitsf(s20));
    h = mixh(h, bitsf(s21));
    h = mixh(h, bitsf(s22));
    h = mixh(h, bitsf(s23));
    h = mixh(h, bitsf(s24));
    h = mixh(h, bitsf(s25));
    h = mixh(h, bitsf(s26));
    h = mixh(h, bitsf(s27));
    h = mixh(h, bitsf(s28));
    h = mixh(h, bitsf(s29));
    h = mixh(h, bitsf(s30));
    h = mixh(h, bitsf(s31));
    h = mixh(h, bitsf(s32));
    h = mixh(h, bitsf(s33));
    return h;
}
// Leaves nonzero bytes in the stack that the loop's frame then takes.
__attribute__((noinline)) static void scribble() {
    volatile U junk[512];
    for (int i = 0; i < 512; ++i)
        junk[i] = 0xa5a5a5a5a5a5a5a5ul;
}
int main() {
    scribble();
    float fx[8];
    fx[0] = (float)0.0;
    fx[1] = (float)1.0;
    fx[2] = (float)3.25;
    fx[3] = (float)1e30;
    fx[4] = (float)-7.5;
    fx[5] = (float)-1.0;
    fx[6] = (float)-1e30;
    fx[7] = (float)0.0;
    return static_cast<int>(f1(fx, 7, (vzero / vzero), 1e30) % 251);
}

