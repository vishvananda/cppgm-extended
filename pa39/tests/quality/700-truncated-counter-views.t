// Counted loops whose int counter the optimizer widens: each unrolled loop
// compares the truncated counter, and its remainder loop starts from it, so
// the truncation is live after the loop while the widened counter goes
// round. Its readers take only its low 32 bits, so it shares the widened
// counter's register: the compare reads W of that register, with no copy
// on each pass.
extern "C" __attribute__((noinline)) long quotients(const long* p, const long* q, const long* r, int count) {
    long s = 0;
    for (int n = 0; n < count; ++n)
        s += p[n] / 5;
    for (int n = 0; n < count; ++n)
        s += q[n] / 7;
    for (int n = 0; n < count; ++n)
        s += r[n] / 9;
    return s;
}

extern "C" __attribute__((noinline)) int weighted(const int* p, const int* q, int count) {
    int s = 0;
    for (int n = 0; n < count; ++n)
        s += p[n] * 3 + (p[n] >> 2);
    for (int n = 0; n < count; ++n)
        s ^= q[n] * 5 - (q[n] >> 3);
    return s;
}
