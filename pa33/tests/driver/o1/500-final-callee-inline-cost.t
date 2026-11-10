// Keep a loop compact when its hinted callee has expanded its own helpers.
// The volatile inputs prevent evaluating the loop during compilation.
unsigned long mix(unsigned long x)
{
    x ^= x << 7; x ^= x >> 11; x += 101;
    x ^= x << 7; x ^= x >> 11; x += 211;
    x ^= x << 7; x ^= x >> 11; x += 307;
    x ^= x << 7; x ^= x >> 11; x += 401;
    x ^= x << 7; x ^= x >> 11; x += 503;
    x ^= x << 7; x ^= x >> 11; x += 601;
    return x;
}

inline unsigned long expanded(unsigned long x)
{
    return mix(mix(mix(mix(x))));
}

__attribute__((noinline)) unsigned long run(unsigned long x, unsigned count)
{
    for (unsigned i = 0; i != count; ++i)
    {
        x = expanded(x) + i;
        x = expanded(x ^ count);
        x ^= x << 11;
        x += count;
        x ^= x >> 17;
        x *= 37;
        x ^= x >> 19;
    }
    return x;
}

volatile unsigned long initial = 1729;
volatile unsigned iterations = 7;

int main()
{
    return run(initial, iterations) != 7210775075916949423ul;
}
