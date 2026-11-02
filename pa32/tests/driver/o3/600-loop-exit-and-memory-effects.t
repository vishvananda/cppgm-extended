// Invariant choices and conditional stores retain the correct final value,
// including zero trips, call paths and values read outside the loop.
__attribute__((noinline)) void alter(unsigned* p, unsigned i) { *p += i + 3; }
__attribute__((noinline)) unsigned run(unsigned* p, unsigned* out, unsigned n, bool flag) {
    unsigned sum = 0;
    for (unsigned i = 0; i < n; ++i) {
        if (flag) sum += p[i]; else sum ^= p[i];
        if (p[i] & 1u) *out = sum;
        if ((i & 7u) == 3u) alter(out, i);
    }
    return sum + *out;
}
__attribute__((noinline)) unsigned locate(unsigned wanted) {
    unsigned values[4] = {2, 4, 6, 8};
    for (unsigned* p = values; p != values + 4; ++p) if (*p == wanted) return p - values;
    return 4;
}
__attribute__((noinline)) unsigned reverse(unsigned wanted) {
    unsigned values[4] = {2, 4, 6, 8};
    for (unsigned* p = values + 4; p != values;) { --p; if (*p == wanted) return p - values; }
    return 4;
}
int main() {
    volatile unsigned start = 3;
    unsigned data[33]; for (unsigned i = 0; i < 33; ++i) data[i] = start + i;
    for (unsigned n = 0; n <= 33; ++n) for (unsigned flag = 0; flag < 2; ++flag) {
        unsigned out = 17, ref = 17, sum = 0;
        for (unsigned i = 0; i < n; ++i) {
            if (flag) sum += data[i]; else sum ^= data[i];
            if (data[i] & 1u) ref = sum;
            if ((i & 7u) == 3u) ref += i + 3;
        }
        if (run(data, &out, n, flag) != sum + ref || out != ref) return 1;
    }
    if (run(0, &data[0], 0, true) != start) return 2;
    for (unsigned k = 0; k < 6; ++k) {
        unsigned wanted = 2 * k, expect = k >= 1 && k <= 4 ? k - 1 : 4;
        if (locate(wanted) != expect || reverse(wanted) != expect) return 3;
    }
    return 0;
}
