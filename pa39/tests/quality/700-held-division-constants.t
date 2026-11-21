// Four generators of one linear congruential sequence advance together. Each
// remainder by the literal becomes a widening multiply by a magic number and
// a multiply-subtract by the divisor; both constants are built once in the
// block that enters the loop and held in registers through it, shared by the
// four remainders, instead of being rebuilt for every remainder on every pass.
extern "C" __attribute__((noinline)) int streams(int* state, int n) {
    int a = state[0], b = state[1], c = state[2], d = state[3];
    for (int i = 0; i < n; ++i) {
        a = (a * 3877 + 29573) % 139968;
        b = (b * 3877 + 29573) % 139968;
        c = (c * 3877 + 29573) % 139968;
        d = (d * 3877 + 29573) % 139968;
    }
    state[0] = a;
    state[1] = b;
    state[2] = c;
    state[3] = d;
    return a ^ b ^ c ^ d;
}

extern "C" int next_seed(int seed);

// The block that enters this loop calls: a constant built at its start must
// stay in a register the call preserves, or be built in the loop.
extern "C" __attribute__((noinline)) int seeded(int seed, int n) {
    int a = next_seed(seed);
    int i = 0;
    do
        a = (a * 3877 + 29573) % 139968;
    while (a != 1 && ++i < n);
    return a;
}
