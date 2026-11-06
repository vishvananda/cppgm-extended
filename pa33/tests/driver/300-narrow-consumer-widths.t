// Narrow results must agree for negative inputs, live wider uses and addresses.
// The checksum was independently checked with GCC and Clang at O0 through O3.
typedef unsigned long long U64;

extern "C" __attribute__((noinline)) U64 shl8(const signed char* p, unsigned shift, const unsigned* table)
{
  unsigned x = *p;
  return x << 13;
}

extern "C" __attribute__((noinline)) U64 add8(const signed char* p, unsigned shift, const unsigned* table)
{
  unsigned x = *p;
  return x + 19U;
}

extern "C" __attribute__((noinline)) U64 sub8(const signed char* p, unsigned shift, const unsigned* table)
{
  unsigned x = *p;
  return x - 91U;
}

extern "C" __attribute__((noinline)) U64 and8(const signed char* p, unsigned shift, const unsigned* table)
{
  unsigned x = *p;
  return x & 0x87654321U;
}

extern "C" __attribute__((noinline)) U64 or8(const signed char* p, unsigned shift, const unsigned* table)
{
  unsigned x = *p;
  return x | 0x87654321U;
}

extern "C" __attribute__((noinline)) U64 xor8(const signed char* p, unsigned shift, const unsigned* table)
{
  unsigned x = *p;
  return x ^ 0x87654321U;
}

extern "C" __attribute__((noinline)) U64 mul8(const signed char* p, unsigned shift, const unsigned* table)
{
  unsigned x = *p;
  return x * 7U;
}

extern "C" __attribute__((noinline)) U64 neg8(const signed char* p, unsigned shift, const unsigned* table)
{
  unsigned x = *p;
  return -x;
}

extern "C" __attribute__((noinline)) U64 not8(const signed char* p, unsigned shift, const unsigned* table)
{
  unsigned x = *p;
  return ~x;
}

extern "C" __attribute__((noinline)) U64 shr8(const signed char* p, unsigned shift, const unsigned* table)
{
  unsigned x = *p;
  return x >> 7;
}

extern "C" __attribute__((noinline)) U64 variable8(const signed char* p, unsigned shift, const unsigned* table)
{
  unsigned x = *p;
  return x << (shift & 31U);
}

extern "C" __attribute__((noinline)) U64 rotate8(const signed char* p, unsigned shift, const unsigned* table)
{
  unsigned x = *p;
  return (x << 11) | (x >> 21);
}

extern "C" __attribute__((noinline)) U64 identity8(const signed char* p, unsigned shift, const unsigned* table)
{
  unsigned x = *p;
  return x;
}

extern "C" __attribute__((noinline)) U64 live8(const signed char* p, unsigned shift, const unsigned* table)
{
  unsigned x = *p;
  return static_cast<U64>(x) + (x << 13);
}

extern "C" __attribute__((noinline)) U64 flags8(const signed char* p, unsigned shift, const unsigned* table)
{
  unsigned x = *p;
  unsigned r = x + 19U;
  return r < 20U ? r : ~r;
}

extern "C" __attribute__((noinline)) U64 memory8(const signed char* p, unsigned shift, const unsigned* table)
{
  unsigned x = *p;
  return (x & 255U) + table[x & 255U];
}

extern "C" __attribute__((noinline)) U64 mul16(const short* p)
{
  unsigned x = *p;
  return x * 7U;
}

extern "C" __attribute__((noinline)) U64 signed64(const signed char* p)
{
  long long x = *p;
  return x * 8192;
}

extern "C" __attribute__((noinline)) U64 address32(U64 wide, const unsigned* table)
{
  unsigned x = wide;
  if (x > 255U) x = 17U;
  return table[x] + static_cast<U64>(x << 13);
}

int main()
{
  unsigned table[256];
  for (unsigned i = 0; i < 256; ++i)
    table[i] = i * 1234567U;
  U64 hash = 1469598103934665603ULL;
  for (unsigned i = 0; i < 4096; ++i)
  {
    signed char byte = static_cast<signed char>(i);
    short word = static_cast<short>(i * 32749U);
    unsigned shift = i % 64U;
    U64 wide = (static_cast<U64>(i * 32179037U) << 32) | (i & 255U);
    hash = (hash ^ shl8(&byte, shift, table)) * 1099511628211ULL;
    hash = (hash ^ add8(&byte, shift, table)) * 1099511628211ULL;
    hash = (hash ^ sub8(&byte, shift, table)) * 1099511628211ULL;
    hash = (hash ^ and8(&byte, shift, table)) * 1099511628211ULL;
    hash = (hash ^ or8(&byte, shift, table)) * 1099511628211ULL;
    hash = (hash ^ xor8(&byte, shift, table)) * 1099511628211ULL;
    hash = (hash ^ mul8(&byte, shift, table)) * 1099511628211ULL;
    hash = (hash ^ neg8(&byte, shift, table)) * 1099511628211ULL;
    hash = (hash ^ not8(&byte, shift, table)) * 1099511628211ULL;
    hash = (hash ^ shr8(&byte, shift, table)) * 1099511628211ULL;
    hash = (hash ^ variable8(&byte, shift, table)) * 1099511628211ULL;
    hash = (hash ^ rotate8(&byte, shift, table)) * 1099511628211ULL;
    hash = (hash ^ identity8(&byte, shift, table)) * 1099511628211ULL;
    hash = (hash ^ live8(&byte, shift, table)) * 1099511628211ULL;
    hash = (hash ^ flags8(&byte, shift, table)) * 1099511628211ULL;
    hash = (hash ^ memory8(&byte, shift, table)) * 1099511628211ULL;
    hash = (hash ^ mul16(&word)) * 1099511628211ULL;
    hash = (hash ^ signed64(&byte)) * 1099511628211ULL;
    hash = (hash ^ address32(wide, table)) * 1099511628211ULL;
  }
  return hash == 0x030196fc79da7403ULL ? 0 : 1;
}
