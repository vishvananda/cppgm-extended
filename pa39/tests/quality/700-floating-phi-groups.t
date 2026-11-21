// Eight lanes of an escape-time loop each carry two floating values through a
// conditional update: a phi at the loop header, a phi where the lane's update
// rejoins, and the updated value. None of the three overlaps another, so the
// first of them to get a SIMD register asks the ranges chosen after it to leave
// that register to the other two, as in the general bank: the loop then needs
// no vector register moves. The outer loop keeps two more floating sums live.
extern "C" __attribute__((noinline)) double row(double ci0, unsigned long width, unsigned char* line) {
    double a0 = 0.25, a1 = 1.25;
    for (unsigned long x = 0; x < (width + 7) / 8; ++x) {
        double cr0[8];
        for (unsigned long k = 0; k < 8; ++k)
            cr0[k] = 2.0 * (8 * x + k) / width - 1.5;
        double cr[8], ci[8];
        for (unsigned long k = 0; k < 8; ++k) {
            cr[k] = cr0[k];
            ci[k] = ci0;
        }
        unsigned char bits = 0;
        for (unsigned long i = 0; i < 50 && bits != 0xFF; ++i) {
            for (unsigned long k = 0; k < 8; ++k) {
                const unsigned char mask = 1 << (7 - k);
                if ((bits & mask) == 0) {
                    const double r2 = cr[k] * cr[k], i2 = ci[k] * ci[k], ri = cr[k] * ci[k];
                    cr[k] = r2 - i2;
                    cr[k] += cr0[k];
                    ci[k] = 2.0 * ri;
                    ci[k] += ci0;
                    if (r2 + i2 > 4.0)
                        bits |= mask;
                }
            }
        }
        line[x] = ~bits;
        a0 = a0 * 0.5 + cr[0];
        a1 = a1 * 0.5 + cr[1];
    }
    return a0 + a1;
}
