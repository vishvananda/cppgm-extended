static unsigned planning_data[512];

extern "C" __attribute__((noinline)) unsigned planned_transform(unsigned seed)
{
    unsigned sum = 0;
    for (unsigned i = 0; i < 64; i += 4)
        for (unsigned lane = 0; lane < 4; ++lane) {
            unsigned x = planning_data[i + lane];
            sum += (x + seed) ^ ((x >> 5) + 11u + lane * 6u);
        }
    return sum;
}

extern "C" __attribute__((noinline)) unsigned planned_matrix(unsigned seed)
{
    unsigned result = 0;
    for (unsigned i = 0; i < 16; ++i)
        for (unsigned j = 0; j < 16; ++j) {
            unsigned sum = seed;
            for (unsigned k = 0; k < 16; ++k)
                sum += planning_data[i * 16 + k] * planning_data[256 + k * 16 + j];
            unsigned shift = (i + j) & 7;
            result ^= (sum << shift) | (sum >> ((32 - shift) & 31));
        }
    return result;
}

// Per-lane scalar shifts have no other vector work to pay for their expansion.
extern "C" __attribute__((noinline)) unsigned shift_only()
{
    unsigned sum = 0;
    for (unsigned long i = 0; i < 64; ++i)
        sum += planning_data[i] << (planning_data[i + 64] & 31);
    return sum;
}

// The outer count is bounded, but adding an unknown unsigned seed can wrap.
extern "C" __attribute__((noinline)) unsigned wrapped_index(unsigned start)
{
    unsigned sum = 0;
    for (unsigned i = 0; i < 64; ++i) {
        unsigned k = start + i;
        if (k < 64) sum += planning_data[k];
    }
    return sum;
}

extern "C" __attribute__((noinline)) unsigned wrapped_counter()
{
    unsigned i = 0xfffffffcu, sum = 0;
    do {
        sum ^= planning_data[i & 63];
        i += 4;
    } while (i < 64);
    return sum;
}

extern "C" unsigned planning_observe(unsigned seed)
{
    unsigned state = seed;
    for (unsigned i = 0; i < 512; ++i) {
        state = state * 1664525u + 1013904223u;
        planning_data[i] = state;
    }
    volatile unsigned* p = planning_data;
    unsigned transform = 0, matrix = 0, shift = 0;
    for (unsigned i = 0; i < 64; ++i) {
        unsigned x = p[i];
        transform += (x + seed) ^ ((x >> 5) + 11u + (i & 3) * 6u);
        shift += p[i] << (p[i + 64] & 31);
    }
    // Visit columns before rows in the reference, and preserve each dot
    // product's scalar order through volatile input reads.
    for (unsigned j = 0; j < 16; ++j)
        for (unsigned i = 0; i < 16; ++i) {
            unsigned sum = seed;
            for (unsigned k = 0; k < 16; ++k)
                sum += p[i * 16 + k] * p[256 + k * 16 + j];
            unsigned n = (i + j) & 7;
            matrix ^= (sum << n) | (sum >> ((32 - n) & 31));
        }
    unsigned starts[] = {0, 32, 64, 0xffffffe0u, 0xffffffffu, seed};
    for (unsigned n = 0; n < 6; ++n) {
        unsigned reference = 0;
        for (unsigned i = 0; i < 64; ++i) {
            unsigned k = starts[n] + i;
            if (k < 64) reference += p[k];
        }
        if (wrapped_index(starts[n]) != reference) return 0;
    }
    unsigned crossed = 0;
    crossed ^= p[60];
    for (unsigned i = 0; i < 64; i += 4) crossed ^= p[i];
    return planned_transform(seed) == transform && planned_matrix(seed) == matrix &&
        shift_only() == shift && wrapped_counter() == crossed ? 17 : 0;
}
