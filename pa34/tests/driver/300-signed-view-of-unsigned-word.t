// A signed view of an unsigned word extends by its own sign, also where the
// word itself is read zero-extended: each reader takes its own type's extension.
// The expected values use 64-bit arithmetic only.
typedef unsigned long long U64;

__attribute__((noinline)) long negated(unsigned x, unsigned y, U64* wide)
{
  unsigned u = -(x + y);
  int s = static_cast<int>(u);
  *wide = u;
  return s;
}

__attribute__((noinline)) long added(unsigned x, unsigned y, U64* wide)
{
  unsigned u = x + y;
  *wide = u;
  return static_cast<int>(u);
}

__attribute__((noinline)) long multiplied(unsigned x, unsigned y, U64* wide)
{
  int s = static_cast<int>(x * y);
  *wide = static_cast<unsigned>(s);
  return s;
}

__attribute__((noinline)) long shifted(unsigned x, unsigned y, U64* wide)
{
  unsigned u = x << (y & 31U);
  long s = static_cast<int>(u);
  *wide = u;
  return s;
}

// An offset back to the entry before, or none: the first entry's is zero.
__attribute__((noinline)) long selected(unsigned x, unsigned y, U64* wide)
{
  unsigned u = x ? -(x + y) : 0;
  int next = static_cast<int>(u);
  *wide = static_cast<U64>(u) << 32;
  return next;
}

__attribute__((noinline)) long merged(unsigned x, unsigned y, U64* wide)
{
  unsigned u;
  if (x & 1U)
    u = x - y;
  else
    u = y * 3U;
  *wide = u;
  return static_cast<int>(u);
}

static U64 low_word(U64 value) { return value & 0xffffffffULL; }

static long signed_word(U64 value)
{
  const U64 word = low_word(value);
  return (word & 0x80000000ULL) ? static_cast<long>(word | 0xffffffff00000000ULL) : static_cast<long>(word);
}

int main()
{
  const unsigned values[] = {0U, 1U, 2U, 3U, 127U, 128U, 255U, 65535U, 65536U, 0x7ffffffeU, 0x7fffffffU,
                             0x80000000U, 0x80000001U, 0xfffffffeU, 0xffffffffU, 0x12345678U, 0x87654321U};
  const unsigned count = sizeof(values) / sizeof(values[0]);
  for (unsigned i = 0; i != count; ++i)
    for (unsigned j = 0; j != count; ++j)
    {
      const U64 x = values[i], y = values[j];
      U64 wide = 0;
      U64 expected = low_word(0ULL - (x + y));
      if (negated(values[i], values[j], &wide) != signed_word(expected) || wide != expected)
        return 1;
      expected = low_word(x + y);
      if (added(values[i], values[j], &wide) != signed_word(expected) || wide != expected)
        return 2;
      expected = low_word(x * y);
      if (multiplied(values[i], values[j], &wide) != signed_word(expected) || wide != expected)
        return 3;
      expected = low_word(x << (y & 31ULL));
      if (shifted(values[i], values[j], &wide) != signed_word(expected) || wide != expected)
        return 4;
      expected = x != 0 ? low_word(0ULL - (x + y)) : 0;
      if (selected(values[i], values[j], &wide) != signed_word(expected) || wide != expected << 32)
        return 5;
      expected = (x & 1ULL) ? low_word(x - y) : low_word(y * 3ULL);
      if (merged(values[i], values[j], &wide) != signed_word(expected) || wide != expected)
        return 6;
    }
  return 0;
}
