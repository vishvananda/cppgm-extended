// Scaled arithmetic keeps its width, wrapped bits and live intermediates.
typedef unsigned long long U64;

template<unsigned Scale>
__attribute__((noinline)) U64 wide(U64 x, U64 y) { return x * Scale + y; }

template<unsigned Scale>
__attribute__((noinline)) U64 narrow(unsigned x, unsigned y)
{
  unsigned value = x * Scale + y;
  return value;
}

template<unsigned Scale>
__attribute__((noinline)) U64 retained(U64 x, U64 y, volatile U64* saved)
{
  U64 product = x * Scale;
  *saved = product;
  return product + y;
}

template<unsigned Scale>
__attribute__((noinline)) U64 conditioned(U64 x, U64 y)
{
  U64 value = x * Scale + y;
  return value < y ? value : ~value;
}

template<unsigned Shift>
__attribute__((noinline)) U64 shifted(U64 x, U64 y) { return (x << Shift) + y; }

template<unsigned Scale>
__attribute__((noinline)) U64 reversed(U64 x, U64 y)
{
  U64 product = x * Scale;
  return y + product;
}

template<unsigned Scale>
__attribute__((noinline)) U64 observed_after(U64 x, U64 y, volatile U64* saved)
{
  U64 product = x * Scale;
  U64 sum = y + product;
  *saved = product;
  return sum;
}

template<unsigned Scale>
__attribute__((noinline)) U64 partially_overwritten(U64 x, U64 y, volatile U64* saved)
{
  U64 product = x * Scale;
  U64 sum = y + product;
  product = (product & ~255ull) | (y & 255ull);
  *saved = product;
  return sum;
}

const U64 inputs[4][2] = {
  {0x0ull, 0x1ull},
  {0x1ull, 0xffffffffffffffffull},
  {0xffffffffffffffffull, 0x8000000000000000ull},
  {0x8123456789abcdefull, 0xfedcba9876543210ull},
};

template<unsigned Scale>
int check(const U64* expected, const unsigned* expected32)
{
  for (unsigned i = 0; i != 4; ++i) {
    U64 x = inputs[i][0], y = inputs[i][1];
    volatile U64 saved = 0;
    if (wide<Scale>(x, y) != expected[i] ||
        narrow<Scale>(static_cast<unsigned>(x), static_cast<unsigned>(y)) != expected32[i] ||
        retained<Scale>(x, y, &saved) != expected[i])
      return 1;
    if (saved + y != expected[i])
      return 2;
    U64 conditional = expected[i] < y ? expected[i] : ~expected[i];
    if (conditioned<Scale>(x, y) != conditional)
      return 3;
    if (reversed<Scale>(x, y) != expected[i] ||
        observed_after<Scale>(x, y, &saved) != expected[i] ||
        saved != expected[i] - y)
      return 4;
    if (partially_overwritten<Scale>(x, y, &saved) != expected[i] ||
        saved != (((expected[i] - y) & ~255ull) | (y & 255ull)))
      return 5;
  }
  return 0;
}

int main()
{
  // Independently evaluated modulo 2^64 and 2^32.
  const U64 expected[][4] = {
    {0x1ull, 0x1ull, 0x7ffffffffffffffeull, 0x123456789abcdeeull},
    {0x1ull, 0x2ull, 0x7ffffffffffffffdull, 0x82468acf13579bddull},
    {0x1ull, 0x3ull, 0x7ffffffffffffffcull, 0x369d0369d0369ccull},
    {0x1ull, 0x5ull, 0x7ffffffffffffffaull, 0x5b05b05b05b05aaull},
    {0x1ull, 0x6ull, 0x7ffffffffffffff9ull, 0x86d3a06d3a06d399ull},
    {0x1ull, 0x7ull, 0x7ffffffffffffff8ull, 0x7f6e5d4c3b2a188ull},
    {0x1ull, 0x8ull, 0x7ffffffffffffff7ull, 0x891a2b3c4d5e6f77ull},
    {0x1ull, 0x9ull, 0x7ffffffffffffff6ull, 0xa3d70a3d70a3d66ull},
    {0x1ull, 0xbull, 0x7ffffffffffffff4ull, 0xc83fb72ea61d944ull},
    {0x1ull, 0x11ull, 0x7fffffffffffffeeull, 0x13579be02468acdeull},
    {0x1ull, 0x13ull, 0x7fffffffffffffecull, 0x159e26af37c048bcull},
    {0x1ull, 0x17ull, 0x7fffffffffffffe8ull, 0x1a2b3c4d5e6f8078ull},
    {0x1ull, 0x23ull, 0x7fffffffffffffdcull, 0x27d27d27d27d27acull},
    {0x1ull, 0x27ull, 0x7fffffffffffffd8ull, 0x2c5f92c5f92c5f68ull},
    {0x1ull, 0x47ull, 0x7fffffffffffffb8ull, 0x50c83fb72ea61d48ull},
  };
  const unsigned expected32[][4] = {
    {0x1u, 0x1u, 0xfffffffeu, 0x89abcdeeu},
    {0x1u, 0x2u, 0xfffffffdu, 0x13579bddu},
    {0x1u, 0x3u, 0xfffffffcu, 0x9d0369ccu},
    {0x1u, 0x5u, 0xfffffffau, 0xb05b05aau},
    {0x1u, 0x6u, 0xfffffff9u, 0x3a06d399u},
    {0x1u, 0x7u, 0xfffffff8u, 0xc3b2a188u},
    {0x1u, 0x8u, 0xfffffff7u, 0x4d5e6f77u},
    {0x1u, 0x9u, 0xfffffff6u, 0xd70a3d66u},
    {0x1u, 0xbu, 0xfffffff4u, 0xea61d944u},
    {0x1u, 0x11u, 0xffffffeeu, 0x2468acdeu},
    {0x1u, 0x13u, 0xffffffecu, 0x37c048bcu},
    {0x1u, 0x17u, 0xffffffe8u, 0x5e6f8078u},
    {0x1u, 0x23u, 0xffffffdcu, 0xd27d27acu},
    {0x1u, 0x27u, 0xffffffd8u, 0xf92c5f68u},
    {0x1u, 0x47u, 0xffffffb8u, 0x2ea61d48u},
  };
  if (check<2>(expected[0], expected32[0])) return 1;
  if (check<3>(expected[1], expected32[1])) return 2;
  if (check<4>(expected[2], expected32[2])) return 3;
  if (check<6>(expected[3], expected32[3])) return 4;
  if (check<7>(expected[4], expected32[4])) return 5;
  if (check<8>(expected[5], expected32[5])) return 6;
  if (check<9>(expected[6], expected32[6])) return 7;
  if (check<10>(expected[7], expected32[7])) return 8;
  if (check<12>(expected[8], expected32[8])) return 9;
  if (check<18>(expected[9], expected32[9])) return 10;
  if (check<20>(expected[10], expected32[10])) return 11;
  if (check<24>(expected[11], expected32[11])) return 12;
  if (check<36>(expected[12], expected32[12])) return 13;
  if (check<40>(expected[13], expected32[13])) return 14;
  if (check<72>(expected[14], expected32[14])) return 15;
  for (unsigned i = 0; i != 4; ++i)
    if (shifted<1>(inputs[i][0], inputs[i][1]) != expected[0][i]) return 21;
  for (unsigned i = 0; i != 4; ++i)
    if (shifted<2>(inputs[i][0], inputs[i][1]) != expected[2][i]) return 22;
  for (unsigned i = 0; i != 4; ++i)
    if (shifted<3>(inputs[i][0], inputs[i][1]) != expected[5][i]) return 23;
  return 0;
}
