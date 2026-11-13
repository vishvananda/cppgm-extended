static unsigned data[83];

extern "C" __attribute__((noinline)) unsigned packed_sum(const unsigned* p, unsigned seed)
{
    unsigned result = seed;
    result += (p[0] + seed) ^ (p[0] >> 5);
    result += (p[1] + seed) ^ (p[1] >> 5);
    result += (p[2] + seed) ^ (p[2] >> 5);
    result += (p[3] + seed) ^ (p[3] >> 5);
    return result;
}

extern "C" __attribute__((noinline)) unsigned sum_less(unsigned n, unsigned threshold)
{
    unsigned result = 9;
    for (unsigned long i = 0; i < n; ++i)
        if (data[i] < threshold) result += data[i] * 7u;
    return result;
}

extern "C" __attribute__((noinline)) unsigned sum_le(unsigned n, unsigned threshold)
{
    unsigned result = 9;
    for (unsigned long i = 0; i < n; ++i)
        if (data[i] <= threshold) result -= data[i] ^ 31u;
    return result;
}

extern "C" __attribute__((noinline)) unsigned sum_greater(unsigned n, unsigned threshold)
{
    unsigned result = 0x12345678u;
    for (unsigned long i = 0; i < n; ++i)
        if (data[i] > threshold) result ^= data[i] + 17u;
    return result;
}

extern "C" __attribute__((noinline)) unsigned sum_ge(unsigned n, unsigned threshold)
{
    unsigned result = 0xffffffffu;
    for (unsigned long i = 0; i < n; ++i)
        if (data[i] >= threshold) result &= data[i] | 0xffu;
    return result;
}

// A predicate depending on the recurrence cannot be evaluated in lanes.
extern "C" __attribute__((noinline)) unsigned dependent(unsigned n)
{
    unsigned result = 9;
    for (unsigned long i = 0; i < n; ++i)
        if (data[i] < result) result += data[i];
    return result;
}

extern "C" __attribute__((noinline)) unsigned updated_condition(unsigned n, unsigned threshold)
{
    unsigned result = 9;
    for (unsigned long i = 0; i < n; ++i) {
        unsigned next = result + data[i];
        if (next < threshold) result = next;
    }
    return result;
}

extern "C" unsigned observe(unsigned seed, unsigned n)
{
    for (unsigned i = 0; i < 83; ++i) {
        seed = seed * 1664525u + 1013904223u;
        data[i] = i % 5 == 0 ? 0x80000000u : i % 5 == 1 ? 0xffffffffu : seed;
    }
    unsigned thresholds[] = {0, 1, 0x7fffffffu, 0x80000000u, 0xffffffffu, seed};
    unsigned packed_reference = seed;
    volatile unsigned* packed_input = data;
    for (unsigned i = 0; i < 4; ++i)
        packed_reference += (packed_input[i] + seed) ^ (packed_input[i] >> 5);
    if (packed_sum(data, seed) != packed_reference) return 0;
    for (unsigned t = 0; t < 6; ++t) {
        unsigned threshold = thresholds[t];
        unsigned a = 9, b = 9, c = 0x12345678u, d = 0xffffffffu, e = 9, f = 9;
        volatile unsigned* reference = data;
        for (unsigned i = 0; i < n; ++i) {
            unsigned x = reference[i];
            if (x < threshold) a += x * 7u;
            if (x <= threshold) b -= x ^ 31u;
            if (x > threshold) c ^= x + 17u;
            if (x >= threshold) d &= x | 0xffu;
            if (x < e) e += x;
            unsigned next = f + x;
            if (next < threshold) f = next;
        }
        if (sum_less(n, threshold) != a || sum_le(n, threshold) != b ||
            sum_greater(n, threshold) != c || sum_ge(n, threshold) != d || dependent(n) != e ||
            updated_condition(n, threshold) != f)
            return 0;
    }
    return 17;
}
