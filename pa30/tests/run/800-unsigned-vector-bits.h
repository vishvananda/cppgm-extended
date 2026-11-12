typedef unsigned Words __attribute__((vector_size(16)));
static unsigned evaluations;
static unsigned lane(unsigned value) { ++evaluations; return value; }
static Words& select_words(Words& value) { ++evaluations; return value; }

extern "C" __attribute__((noinline)) Words vector_step(Words x, unsigned seed, unsigned shift)
{
    return ((x * Words{3u, 5u, 7u, 11u}) + seed) ^ (x >> shift);
}

extern "C" __attribute__((noinline)) unsigned vector_sum(Words x)
{
    return x[0] + x[1] + x[2] + x[3];
}

extern "C" unsigned observe(unsigned seed, unsigned count)
{
    evaluations = 0;
    Words x = {lane(seed), lane(seed ^ 0x80000000u), lane(~seed), lane(seed + 12345u)};
    if (evaluations != 4u) return 1;
    volatile Words saved = x;
    x = saved;
    unsigned expected[4] = {seed, seed ^ 0x80000000u, ~seed, seed + 12345u};
    const unsigned factor[4] = {3, 5, 7, 11};
    for (unsigned i = 0; i < count; ++i) {
        unsigned shift = (seed + i) & 31u;
        x = vector_step(x, lane(seed + i), shift);
        for (unsigned k = 0; k < 4; ++k)
            expected[k] = (expected[k] * factor[k] + seed + i) ^ (expected[k] >> shift);
    }
    if (evaluations != 4u + count) return 2;
    for (unsigned k = 0; k < 4; ++k)
        if (x[k] != expected[k]) return 3;
    if (vector_sum(x) != expected[0] + expected[1] + expected[2] + expected[3]) return 4;
    if (select_words(x)[2] != expected[2] || evaluations != 5u + count) return 6;
    saved = x;
    if (saved[2] != expected[2]) return 7;
    x[2] ^= 0x80000000u;
    expected[2] ^= 0x80000000u;
    saved = x + 0x80000000u;
    x = saved;
    return vector_sum(x) == expected[0] + expected[1] + expected[2] + expected[3] ? 17 : 5;
}
