// Values a loop computes and code after it reads: sums, counters and the last
// pass's value, at trip counts from 0 to 70 (lane multiples and not), from
// zero and nonzero starts, forward and backward, through a pointer, in
// memory, in narrow types, from an inner loop into an outer one, and in loops
// that also store, whose arrays may overlap.
unsigned words[512] __attribute__((aligned(16)));
int ints[512] __attribute__((aligned(16)));
unsigned char bytes[512] __attribute__((aligned(16)));
unsigned short halves[512] __attribute__((aligned(16)));
unsigned long long longs[512] __attribute__((aligned(16)));
unsigned memory_sum;

void fill(unsigned seed) {
    for (unsigned i = 0; i < 512; ++i) {
        seed = seed * 1664525u + 1013904223u;
        words[i] = seed;
        ints[i] = static_cast<int>(seed >> 8) - (1 << 23);
        bytes[i] = static_cast<unsigned char>(seed >> 13);
        halves[i] = static_cast<unsigned short>(seed >> 7);
        longs[i] = (static_cast<unsigned long long>(seed) << 29) ^ seed;
    }
}
__attribute__((noinline)) unsigned sum_from_zero(unsigned n) {
    unsigned s = 0;
    for (unsigned i = 0; i < n; ++i) s += words[i];
    return s;
}
__attribute__((noinline)) unsigned sum_from_seed(unsigned n, unsigned seed) {
    unsigned s = seed;
    for (unsigned i = 0; i < n; ++i) s += words[i] * 3u;
    return s ^ (s >> 7);
}
__attribute__((noinline)) int two_sums(int n) {
    int s = 5, x = 0;
    for (int i = 0; i < n; ++i) { s += ints[i]; x ^= ints[i] * 2; }
    return s + x * 3;
}
__attribute__((noinline)) unsigned backward(unsigned n) {
    unsigned s = 11;
    for (unsigned i = n; i-- > 0;) s += words[i] ^ i;
    return s;
}
__attribute__((noinline)) unsigned long long counter_and_sum(unsigned long long n) {
    unsigned long long s = 0, i = 0;
    for (; i < n; ++i) s += longs[i];
    return s + i * 1000003u;
}
__attribute__((noinline)) unsigned pointer_counter(unsigned n) {
    unsigned* p = words;
    unsigned* end = words + n;
    unsigned s = 0;
    for (; p != end; ++p) s += *p;
    return s + static_cast<unsigned>(p - words);
}
__attribute__((noinline)) unsigned last_value(unsigned n) {
    unsigned s = 0, last = 0;
    for (unsigned i = 0; i < n; ++i) { last = words[i] * 5u; s += last; }
    return s ^ last;
}
__attribute__((noinline)) unsigned conditional(unsigned n, unsigned limit) {
    unsigned s = 1;
    for (unsigned i = 0; i < n; ++i)
        if ((words[i] & 1023u) < limit) s += words[i] ^ limit;
    return s;
}
__attribute__((noinline)) unsigned in_memory(unsigned n) {
    for (unsigned i = 0; i < n; ++i) memory_sum += words[i];
    return memory_sum;
}
__attribute__((noinline)) unsigned narrow(unsigned n) {
    unsigned char b = 3;
    unsigned short h = 9;
    for (unsigned i = 0; i < n; ++i) { b += bytes[i]; h += halves[i]; }
    return b * 65536u + h;
}
__attribute__((noinline)) unsigned guarded(unsigned* out, const unsigned* in, unsigned n) {
    unsigned s = 0;
    for (unsigned i = 0; i < n; ++i) { out[i] = in[i] + 1u; s += in[i]; }
    return s;
}
__attribute__((noinline)) unsigned cells_and_sum(unsigned n) {
    unsigned cells[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    unsigned s = 0;
    for (unsigned i = 0; i < n; ++i) { cells[i % 8] = cells[i % 8] * 33u + words[i]; s += words[i]; }
    unsigned h = s;
    for (unsigned k = 0; k < 8; ++k) h = h * 31u + cells[k];
    return h;
}
// Literal trip counts: whole vector rounds, and not.
__attribute__((noinline)) unsigned literal_even(unsigned seed) {
    unsigned s = seed;
    for (unsigned i = 0; i < 32; ++i) s += words[i] * words[i + 64];
    return s;
}
__attribute__((noinline)) unsigned literal_odd(unsigned seed) {
    unsigned s = seed;
    for (unsigned i = 0; i < 35; ++i) s ^= words[i] + words[i + 64];
    return s;
}
__attribute__((noinline)) unsigned literal_backward(unsigned seed) {
    unsigned s = seed;
    for (int i = 31; i >= 0; --i) s += words[i] ^ static_cast<unsigned>(i);
    return s;
}
__attribute__((noinline)) unsigned guarded_literal(unsigned* out, const unsigned* in) {
    unsigned s = 0;
    for (unsigned i = 0; i < 64; ++i) { out[i] = in[i] * 3u; s ^= in[i]; }
    return s;
}
// An inner loop's sum reaches the outer loop's phi.
__attribute__((noinline)) unsigned matrix(unsigned seed) {
    unsigned result = 0;
    for (unsigned i = 0; i < 16; ++i)
        for (unsigned j = 0; j < 16; ++j) {
            unsigned sum = seed;
            for (unsigned k = 0; k < 16; ++k) sum += words[i * 16 + k] * words[256 + k * 16 + j];
            unsigned shift = (i + j) & 7;
            result ^= (sum << shift) | (sum >> ((32 - shift) & 31));
        }
    return result;
}

unsigned long long hash;
void mix(unsigned long long v) { hash = (hash ^ v) * 1099511628211ull; }

int main() {
    static const unsigned long long expected[3] = { 0x2a2cbb1f24e837deull, 0xe91a3ce0380925b0ull, 0xbb85261748498c52ull };
    for (unsigned seed = 1; seed < 4; ++seed) {
        hash = 14695981039346656037ull;
        memory_sum = 0;
        fill(seed);
        for (unsigned n = 0; n < 71; ++n) {
            mix(sum_from_zero(n)); mix(sum_from_seed(n, seed * 77)); mix(static_cast<unsigned>(two_sums(static_cast<int>(n))));
            mix(backward(n)); mix(counter_and_sum(n)); mix(pointer_counter(n)); mix(last_value(n));
            mix(conditional(n, 300 + seed)); mix(in_memory(n)); mix(narrow(n)); mix(cells_and_sum(n));
            mix(guarded(words + 300, words + 100, n)); mix(guarded(words + 101, words + 100, n));
            mix(guarded(words + 100, words + 103, n));
        }
        mix(literal_even(seed)); mix(literal_odd(seed)); mix(literal_backward(seed));
        mix(guarded_literal(words + 300, words + 100)); mix(guarded_literal(words + 102, words + 100));
        mix(guarded_literal(words + 100, words + 101)); mix(matrix(seed));
        if (hash != expected[seed - 1]) return static_cast<int>(seed);
    }
    return 0;
}
