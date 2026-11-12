static unsigned input[2048] __attribute__((aligned(16)));

extern "C" void prepare(unsigned seed)
{
    for (unsigned i = 0; i < 2048; ++i) {
        seed = seed * 1664525u + 1013904223u;
        input[i] = seed;
    }
}

static __attribute__((noinline)) unsigned hash_words(unsigned seed)
{
    for (unsigned i = 0; i < 2048; ++i) {
        seed ^= input[i];
        seed = (seed << 7) | (seed >> 25);
        seed = seed * 16777619u + i;
    }
    return seed;
}

static __attribute__((noinline)) unsigned filtered_sum(unsigned seed)
{
    unsigned result = 0;
    for (unsigned i = 0; i < 2048; ++i)
        if ((input[i] & 1023u) < (seed & 1023u)) result += input[i] ^ seed;
    return result;
}

static __attribute__((noinline)) unsigned matrix_product(unsigned seed)
{
    unsigned result = 0;
    for (unsigned i = 0; i < 16; ++i)
        for (unsigned j = 0; j < 16; ++j) {
            unsigned sum = seed;
            for (unsigned k = 0; k < 16; ++k)
                sum += input[i * 16 + k] * input[256 + k * 16 + j];
            unsigned shift = (i + j) & 7;
            result ^= (sum << shift) | (sum >> ((32 - shift) & 31));
        }
    return result;
}

static __attribute__((noinline)) unsigned transform_words(unsigned seed)
{
#if CPPGM_BENCH_SIMD
    typedef unsigned Words __attribute__((vector_size(16)));
    Words result = {0,0,0,0};
    for (unsigned i = 0; i < 2048; i += 4) {
        Words x = *reinterpret_cast<Words*>(input + i);
        Words value = (x * Words{3,3,3,3} + Words{seed,seed,seed,seed}) ^
            ((x >> 5) + Words{11,17,23,29});
        result += value;
    }
    return result[0] + result[1] + result[2] + result[3];
#else
    unsigned result = 0;
    for (unsigned i = 0; i < 2048; i += 4) {
        for (unsigned lane = 0; lane < 4; ++lane) {
            unsigned x = input[i+lane];
            result += (x * 3u + seed) ^ ((x >> 5) + 11u + lane * 6u);
        }
    }
    return result;
#endif
}

extern "C" unsigned run(unsigned kind, unsigned repeats, unsigned seed)
{
    unsigned result = 17;
    for (unsigned r = 0; r < repeats; ++r) {
        unsigned value = kind == 0 ? hash_words(seed+r) : kind == 1 ? filtered_sum(seed+r)
            : kind == 2 ? matrix_product(seed+r) : transform_words(seed+r);
        result = ((result << 5) | (result >> 27)) ^ value;
    }
    return result;
}
