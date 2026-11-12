static volatile unsigned visits;

extern "C" __attribute__((noinline)) unsigned tick(unsigned i, unsigned value)
{
    ++visits;
    return value ^ (i * 11u + visits);
}

extern "C" __attribute__((noinline)) unsigned call_loop(unsigned count, unsigned a, unsigned b)
{
    unsigned result = 0;
    for (unsigned i = 0; i < count; ++i)
        result += tick(i, (a ^ 31u) + (b << 2));
    return result;
}

extern "C" unsigned observe(unsigned count, unsigned a, unsigned b)
{
    visits = 0;
    unsigned actual = call_loop(count, a, b);
    unsigned expected = 0;
    unsigned value = (a ^ 31u) + (b << 2);
    for (unsigned i = 0; i < count; ++i)
        expected += value ^ (i * 11u + i + 1u);
    return actual == expected && visits == count ? 17 : 1;
}
