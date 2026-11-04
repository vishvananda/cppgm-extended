volatile unsigned narrow_input = 0xfffffff0u;
volatile unsigned long wide_input = 0xfffffffffffffff0ul;

__attribute__((noinline)) unsigned opaque(unsigned value)
{
  return value ^ 0x13579bdu;
}

__attribute__((noinline)) unsigned narrow(unsigned a, unsigned b, unsigned c,
    unsigned d, unsigned e, unsigned f)
{
  unsigned x = a + b, y = c + d, z = e + f;
  unsigned keep = opaque(a ^ c ^ e);
  return (x + y) ^ (z + keep) ^ (a + f) ^ (b + e) ^ (c + d);
}

__attribute__((noinline)) unsigned long wide(unsigned long a, unsigned long b,
    unsigned long c, unsigned long d, unsigned long e, unsigned long f)
{
  unsigned long x = a + b, y = c + d, z = e + f;
  unsigned long keep = opaque(static_cast<unsigned>(a ^ c ^ e));
  return (x + y) ^ (z + keep) ^ (a + f) ^ (b + e) ^ (c + d);
}

int main()
{
  narrow_input = 0u;
  if (narrow(narrow_input + 0u, narrow_input + 7u, narrow_input + 14u, narrow_input + 21u, narrow_input + 28u, narrow_input + 35u) != 20281831u) return 1;
  narrow_input = 4294967280u;
  if (narrow(narrow_input + 0u, narrow_input + 7u, narrow_input + 14u, narrow_input + 21u, narrow_input + 28u, narrow_input + 35u) != 4274685495u) return 1;
  wide_input = 0ul;
  if (wide(wide_input + 0ul, wide_input + 7ul, wide_input + 14ul, wide_input + 21ul, wide_input + 28ul, wide_input + 35ul) != 20281831ul) return 1;
  wide_input = 18446744073709551600ul;
  if (wide(wide_input + 0ul, wide_input + 7ul, wide_input + 14ul, wide_input + 21ul, wide_input + 28ul, wide_input + 35ul) != 18446744073689269815ul) return 1;
  return 0;
}
