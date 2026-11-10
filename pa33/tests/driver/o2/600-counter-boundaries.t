// Arithmetic identities preserve signed rounding, wrapping unsigned values,
// narrowed results, multiple latches and values used after the loop.
__attribute__((noinline)) unsigned chain(unsigned v, unsigned n) {
    for (unsigned i = 0; i < n; ++i) v = (v * 33u + i) ^ (v >> 3);
    return v;
}
__attribute__((noinline)) int quotients(int x, int y) { return (x / y) * y + x % y; }
__attribute__((noinline)) unsigned multi(unsigned* p, unsigned n, unsigned stride) {
    unsigned sum = 0, i = 0;
    while (i < n) { unsigned k = i * stride;
        if (p[i] & 1u) { sum += k; ++i; continue; }
        sum ^= k; ++i;
    }
    return sum + i;
}
__attribute__((noinline)) unsigned long long wide(unsigned start, unsigned n) {
    unsigned long long sum = 0;
    for (unsigned i = 0; i < n; ++i) sum += static_cast<unsigned long long>(start + i);
    return sum;
}
int main() {
    volatile unsigned seed = 0xfffffff0u;
    unsigned state = seed, expect = seed;
    for (unsigned n = 0; n < 33; ++n) {
        state = chain(seed, n); expect = seed;
        for (unsigned i = 0; i < n; ++i) expect = (expect * 33u + i) ^ (expect >> 3);
        if (state != expect) return 1;
    }
    int xs[] = {-2147483647, -19, -1, 0, 1, 19, 2147483647};
    int ys[] = {-7, -1, 1, 7};
    for (unsigned i = 0; i < 7; ++i) for (unsigned j = 0; j < 4; ++j)
        if (quotients(xs[i], ys[j]) != xs[i]) return 2;
    unsigned data[17]; for (unsigned i = 0; i < 17; ++i) data[i] = i;
    for (unsigned n = 0; n < 18; ++n) {
        unsigned ref = 0;
        for (unsigned i = 0; i < n; ++i) if (i & 1u) ref += i * 13u; else ref ^= i * 13u;
        if (multi(data, n, 13) != ref + n) return 3;
        unsigned long long sum = 0;
        for (unsigned i = 0; i < n; ++i) sum += static_cast<unsigned>(seed + i);
        if (wide(seed, n) != sum) return 4;
    }
    return 0;
}
