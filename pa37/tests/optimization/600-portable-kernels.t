extern "C" unsigned scalar(unsigned a, unsigned b)
{
    return (a * 7 + (b ^ 31)) ^ ((a - b) * 3);
}

extern "C" __attribute__((noinline)) unsigned recurrence(unsigned count)
{
    unsigned result = 17;
    for (unsigned i = 0; i < count; ++i)
        result = (result ^ (i + 3)) * 1664525u + 1013904223u;
    return result;
}

extern "C" __attribute__((noinline)) void transform(unsigned* out, const unsigned* in, unsigned count)
{
    for (unsigned i = 0; i < count; ++i)
        out[i] = in[i] * 3u + 7u;
}

extern "C" unsigned observe(unsigned count)
{
    unsigned input[67], output[67];
    if (count > 67) return 1;
    for (unsigned i = 0; i < count; ++i) input[i] = i * i + 13;
    transform(output, input, count);
    for (unsigned i = 0; i < count; ++i)
        if (output[i] != (i * i + 13) * 3 + 7) return 2;
    return 17;
}
