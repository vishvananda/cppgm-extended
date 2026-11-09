#include <cstdio>
extern "C" int computed_dispatch(int, int);
extern "C" int computed_templates(int);
extern "C" const void *computed_identity(int);
extern "C" unsigned computed_live_state(unsigned, unsigned);
extern "C" int computed_empty_entries(unsigned);
extern "C" unsigned host_mix(unsigned value) { return (value ^ 0x93ad7b51u) + 0x8357u; }
static unsigned expected_state(unsigned state, unsigned rounds) {
    for (unsigned i = 0; i < rounds; ++i) {
        unsigned local;
        if (!(state & 1)) {
            local = state * 17 + 3;
            if (local & 16) local ^= 0x7351;
            else local += 29;
        } else {
            local = state + 0x12345;
            if (local & 8) local *= 3;
            else local ^= 0x9537;
        }
        state = host_mix(local);
    }
    return state;
}
int main() {
    for (unsigned entry = 0; entry < 3; ++entry)
        if (computed_empty_entries(entry) != 71) return 6;
    for (int n = 0; n < 40; ++n) {
        int sum = n * (n + 1) / 2;
        if (computed_dispatch(0, n) != 3 + sum || computed_dispatch(1, n) != sum ||
            computed_dispatch(2, n) != 0) return 1;
    }
    for (unsigned seed = 0; seed < 32; ++seed)
        for (unsigned rounds = 0; rounds < 100; ++rounds)
            if (computed_live_state(seed * 0x139ad1u, rounds) != expected_state(seed * 0x139ad1u, rounds)) return 5;
    if (computed_templates(0) != 311 || computed_templates(1) != 1018) return 3;
    const void *first = computed_identity(0), *second = computed_identity(1);
    if (!first || !second || first != computed_identity(0) || second != computed_identity(1)) return 4;
    std::puts("computed dispatch, templates and code identity passed");
}
