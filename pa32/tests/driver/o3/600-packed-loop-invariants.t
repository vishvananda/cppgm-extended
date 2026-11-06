// Packing adjacent work must preserve zero-trip guards, alias observations,
// volatile input, throwing callbacks and signed zero.
struct Pair { double a, b; };
volatile double factor = 0.5;
volatile unsigned callbacks = 0, destroyed = 0;
struct Guard { ~Guard() { ++destroyed; } };
__attribute__((noinline)) unsigned scale_pair(Pair* p, const volatile double* seen,
                                             unsigned n, double scale) {
    unsigned sum = 0;
    for (unsigned i = 0; i < n; ++i) {
        p->a *= scale; p->b *= scale;
        sum += *seen > 0.0;
    }
    return sum;
}
__attribute__((noinline)) void change(Pair* p, unsigned i, unsigned mode) {
    ++callbacks; factor = 0.25; p->b += i + 2;
    if (mode == 2 && i == 1) throw 5;
}
__attribute__((noinline)) unsigned guarded_pair(Pair* p, unsigned n,
                                               unsigned denominator, unsigned mode) {
    Guard guard;
    unsigned sum = 0;
    for (unsigned i = 0; i < n; ++i) {
        sum += 12 / denominator;
        const double scale = factor;
        p->a *= scale; p->b *= scale;
        sum += p->b > 0.0;
        change(p, i, mode);
    }
    return sum;
}
int main() {
    if (scale_pair(0, 0, 0, 0.5) || guarded_pair(0, 0, 0, 0)) return 1;
    if (callbacks || destroyed != 1) return 2;
    for (unsigned n = 0; n <= 12; ++n) {
        Pair p = {8.0, -16.0};
        double expected = 8.0;
        for (unsigned i = 0; i < n; ++i) expected *= 0.5;
        if (scale_pair(&p, &p.a, n, 0.5) != n || p.a != expected || p.b != -2 * expected) return 3;
        p.a = -0.0; p.b = 0.0;
        if (scale_pair(&p, &p.b, n, 0.5) || 1.0 / p.a >= 0.0 || 1.0 / p.b <= 0.0) return 4;
    }
    Pair p = {8.0, 16.0}; factor = 0.5; callbacks = destroyed = 0;
    if (guarded_pair(&p, 3, 2, 1) != 21 || p.a != 0.25 || p.b != 5.375) return 5;
    if (callbacks != 3 || destroyed != 1) return 6;
    p.a = 8.0; p.b = 16.0; factor = 0.5; callbacks = destroyed = 0;
    try { guarded_pair(&p, 3, 2, 2); return 7; }
    catch (int value) { if (value != 5) return 8; }
    if (callbacks != 2 || destroyed != 1 || p.a != 1.0 || p.b != 5.5) return 9;
    volatile double zero = 0.0;
    const double inf = 1.0 / zero;
    p.a = inf; p.b = inf - inf;
    if (scale_pair(&p, &p.b, 3, 0.5) || p.a != inf || p.b == p.b) return 10;
    return 0;
}
