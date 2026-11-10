// Large weak helpers may be shared across translation units. Bound caller growth,
// while allowing any implementation that preserves the mixing operation.
typedef unsigned long U;
inline U shared_mix(U x)
{
    x = (x ^ (x >> 7)) * 0x9e3779b97f4a7c15UL;
    x = (x ^ (x >> 8)) * 0x9e3779b97f4a7c15UL;
    x = (x ^ (x >> 9)) * 0x9e3779b97f4a7c15UL;
    x = (x ^ (x >> 10)) * 0x9e3779b97f4a7c15UL;
    x = (x ^ (x >> 11)) * 0x9e3779b97f4a7c15UL;
    x = (x ^ (x >> 12)) * 0x9e3779b97f4a7c15UL;
    x = (x ^ (x >> 13)) * 0x9e3779b97f4a7c15UL;
    x = (x ^ (x >> 14)) * 0x9e3779b97f4a7c15UL;
    x = (x ^ (x >> 15)) * 0x9e3779b97f4a7c15UL;
    x = (x ^ (x >> 16)) * 0x9e3779b97f4a7c15UL;
    x = (x ^ (x >> 17)) * 0x9e3779b97f4a7c15UL;
    x = (x ^ (x >> 18)) * 0x9e3779b97f4a7c15UL;
    x = (x ^ (x >> 19)) * 0x9e3779b97f4a7c15UL;
    x = (x ^ (x >> 20)) * 0x9e3779b97f4a7c15UL;
    x = (x ^ (x >> 21)) * 0x9e3779b97f4a7c15UL;
    x = (x ^ (x >> 22)) * 0x9e3779b97f4a7c15UL;
    x = (x ^ (x >> 23)) * 0x9e3779b97f4a7c15UL;
    x = (x ^ (x >> 7)) * 0x9e3779b97f4a7c15UL;
    x = (x ^ (x >> 8)) * 0x9e3779b97f4a7c15UL;
    x = (x ^ (x >> 9)) * 0x9e3779b97f4a7c15UL;
    x = (x ^ (x >> 10)) * 0x9e3779b97f4a7c15UL;
    x = (x ^ (x >> 11)) * 0x9e3779b97f4a7c15UL;
    x = (x ^ (x >> 12)) * 0x9e3779b97f4a7c15UL;
    x = (x ^ (x >> 13)) * 0x9e3779b97f4a7c15UL;
    x = (x ^ (x >> 14)) * 0x9e3779b97f4a7c15UL;
    x = (x ^ (x >> 15)) * 0x9e3779b97f4a7c15UL;
    x = (x ^ (x >> 16)) * 0x9e3779b97f4a7c15UL;
    x = (x ^ (x >> 17)) * 0x9e3779b97f4a7c15UL;
    x = (x ^ (x >> 18)) * 0x9e3779b97f4a7c15UL;
    x = (x ^ (x >> 19)) * 0x9e3779b97f4a7c15UL;
    x = (x ^ (x >> 20)) * 0x9e3779b97f4a7c15UL;
    x = (x ^ (x >> 21)) * 0x9e3779b97f4a7c15UL;
    x = (x ^ (x >> 22)) * 0x9e3779b97f4a7c15UL;
    x = (x ^ (x >> 23)) * 0x9e3779b97f4a7c15UL;
    x = (x ^ (x >> 7)) * 0x9e3779b97f4a7c15UL;
    x = (x ^ (x >> 8)) * 0x9e3779b97f4a7c15UL;
    x = (x ^ (x >> 9)) * 0x9e3779b97f4a7c15UL;
    x = (x ^ (x >> 10)) * 0x9e3779b97f4a7c15UL;
    x = (x ^ (x >> 11)) * 0x9e3779b97f4a7c15UL;
    x = (x ^ (x >> 12)) * 0x9e3779b97f4a7c15UL;
    x = (x ^ (x >> 13)) * 0x9e3779b97f4a7c15UL;
    x = (x ^ (x >> 14)) * 0x9e3779b97f4a7c15UL;
    x = (x ^ (x >> 15)) * 0x9e3779b97f4a7c15UL;
    x = (x ^ (x >> 16)) * 0x9e3779b97f4a7c15UL;
    x = (x ^ (x >> 17)) * 0x9e3779b97f4a7c15UL;
    x = (x ^ (x >> 18)) * 0x9e3779b97f4a7c15UL;
    x = (x ^ (x >> 19)) * 0x9e3779b97f4a7c15UL;
    x = (x ^ (x >> 20)) * 0x9e3779b97f4a7c15UL;

    return x;
}
extern "C" __attribute__((noinline)) U caller(U x) { return shared_mix(x); }
int main()
{
    U state = 0;
    for (unsigned i = 0; i < 1024; ++i)
    {
        state = state * 6364136223846793005UL + 1;
        U expected = state;
        for (unsigned j = 0; j < 48; ++j)
            expected = (expected ^ (expected >> (7 + j % 17))) * 0x9e3779b97f4a7c15UL;
        if (caller(state) != expected) return 1;
    }
    return 0;
}
