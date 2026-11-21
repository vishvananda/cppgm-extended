// More floating values live through a call-free loop than there are SIMD
// registers, beside 18 integer sums that fill the general registers. A
// floating value that finds no SIMD register takes a general register only
// after the integers have chosen: in a general register it still costs a
// move at each use, which an integer saves outright. The loop also carries
// more floating phis than the SIMD registers hold; the input `y`, read by
// every update, keeps its register ahead of them. (Separate statements keep
// Clang from contracting the multiply and the add.)
extern "C" __attribute__((noinline)) long crowded(const long* a, const double* x, int n, double* out) {
    long s0 = 1, s1 = 2, s2 = 3, s3 = 4, s4 = 5, s5 = 6, s6 = 7, s7 = 8, s8 = 9,
        s9 = 10, s10 = 11, s11 = 12, s12 = 13, s13 = 14, s14 = 15, s15 = 16, s16 = 17, s17 = 18;
    double f0 = 1.5, f1 = 2.5, f2 = 3.5, f3 = 4.5, f4 = 5.5, f5 = 6.5, f6 = 7.5, f7 = 8.5, f8 = 9.5,
        f9 = 10.5, f10 = 11.5, f11 = 12.5, f12 = 13.5, f13 = 14.5, f14 = 15.5, f15 = 16.5, f16 = 17.5, f17 = 18.5,
        f18 = 19.5, f19 = 20.5, f20 = 21.5, f21 = 22.5, f22 = 23.5, f23 = 24.5, f24 = 25.5, f25 = 26.5, f26 = 27.5,
        f27 = 28.5, f28 = 29.5, f29 = 30.5, f30 = 31.5, f31 = 32.5, f32 = 33.5, f33 = 34.5;
    for (int i = 0; i < n; ++i) {
        const long v = a[i];
        const double y = x[i];
        s0 += v ^ s1;
        s1 += v ^ s2;
        s2 += v ^ s3;
        s3 += v ^ s4;
        s4 += v ^ s5;
        s5 += v ^ s6;
        s6 += v ^ s7;
        s7 += v ^ s8;
        s8 += v ^ s9;
        s9 += v ^ s10;
        s10 += v ^ s11;
        s11 += v ^ s12;
        s12 += v ^ s13;
        s13 += v ^ s14;
        s14 += v ^ s15;
        s15 += v ^ s16;
        s16 += v ^ s17;
        s17 += v ^ s0;
        f0 = f0 * y;
        f0 += f1;
        f1 = f1 * y;
        f1 += f2;
        f2 = f2 * y;
        f2 += f3;
        f3 = f3 * y;
        f3 += f4;
        f4 = f4 * y;
        f4 += f5;
        f5 = f5 * y;
        f5 += f6;
        f6 = f6 * y;
        f6 += f7;
        f7 = f7 * y;
        f7 += f8;
        f8 = f8 * y;
        f8 += f9;
        f9 = f9 * y;
        f9 += f10;
        f10 = f10 * y;
        f10 += f11;
        f11 = f11 * y;
        f11 += f12;
        f12 = f12 * y;
        f12 += f13;
        f13 = f13 * y;
        f13 += f14;
        f14 = f14 * y;
        f14 += f15;
        f15 = f15 * y;
        f15 += f16;
        f16 = f16 * y;
        f16 += f17;
        f17 = f17 * y;
        f17 += f18;
        f18 = f18 * y;
        f18 += f19;
        f19 = f19 * y;
        f19 += f20;
        f20 = f20 * y;
        f20 += f21;
        f21 = f21 * y;
        f21 += f22;
        f22 = f22 * y;
        f22 += f23;
        f23 = f23 * y;
        f23 += f24;
        f24 = f24 * y;
        f24 += f25;
        f25 = f25 * y;
        f25 += f26;
        f26 = f26 * y;
        f26 += f27;
        f27 = f27 * y;
        f27 += f28;
        f28 = f28 * y;
        f28 += f29;
        f29 = f29 * y;
        f29 += f30;
        f30 = f30 * y;
        f30 += f31;
        f31 = f31 * y;
        f31 += f32;
        f32 = f32 * y;
        f32 += f33;
        f33 = f33 * y;
        f33 += f0;
    }
    out[0] = f0 + f1 + f2 + f3 + f4 + f5 + f6 + f7 + f8 + f9 + f10 + f11 +
        f12 + f13 + f14 + f15 + f16 + f17 + f18 + f19 + f20 + f21 + f22 + f23 +
        f24 + f25 + f26 + f27 + f28 + f29 + f30 + f31 + f32 + f33;
    return s0 + s1 + s2 + s3 + s4 + s5 + s6 + s7 + s8 +
        s9 + s10 + s11 + s12 + s13 + s14 + s15 + s16 + s17;
}
