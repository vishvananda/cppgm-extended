// Integer-wide intermediates and actual floating conversions retain live state.
typedef unsigned long long U64;
typedef unsigned __int128 U128;

__attribute__((noinline)) U64 integer_path(
    U64 a, U64 b, volatile U64* state, const unsigned char* bytes, unsigned count)
{
  U64 s0 = state[0], s1 = state[1], s2 = state[2], s3 = state[3];
  U64 s4 = state[4], s5 = state[5], s6 = state[6], s7 = state[7];
  U128 product = static_cast<U128>(a) * static_cast<U128>(b);
  U64 result = static_cast<U64>(product >> 64);
  const unsigned char* end = bytes + count;
  while (bytes != end) {
    result = result * 33 + *bytes;
    ++bytes;
  }
  result = (result ^ s0) + s1;
  result = (result ^ s2) + s3;
  result = (result ^ s4) + s5;
  return (result ^ s6) + s7;
}

// Independent four-part multiplication, without a wide integer intermediate.
U64 high_product(U64 a, U64 b)
{
  const U64 mask = 0xffffffffull;
  U64 low = (a & mask) * (b & mask);
  U64 middle = (a >> 32) * (b & mask) + (low >> 32);
  U64 upper = middle >> 32;
  middle = (a & mask) * (b >> 32) + (middle & mask);
  return (a >> 32) * (b >> 32) + upper + (middle >> 32);
}

__attribute__((noinline)) U64 floating_path(U128 value, volatile U64* state)
{
  U64 s0 = state[0], s1 = state[1], s2 = state[2], s3 = state[3];
  double converted = static_cast<double>(value);
  U128 restored = static_cast<U128>(converted);
  U64 result = static_cast<U64>(restored) ^ static_cast<U64>(restored >> 64);
  return ((result ^ s0) + s1) ^ (s2 + s3);
}

__attribute__((noinline)) U64 signed_path(long long value, volatile U64* state)
{
  U64 saved = state[0];
  __int128 extended = static_cast<__int128>(value);
  return static_cast<U64>(extended >> 64) ^ static_cast<U64>(extended) ^ saved;
}

int main()
{
  volatile U64 state[8] = {
    0, 1, 0xffffffffffffffffull, 0x8000000000000000ull,
    0x123456789abcdef0ull, 17, 0x8000000000000001ull, 31
  };
  const unsigned char bytes[] = {0, 1, 255, 128, 9, 37, 0, 250};
  const U64 inputs[] = {
    0, 1, 0xffffffffull, 0x100000000ull,
    0x8000000000000000ull, 0xffffffffffffffffull,
    0x123456789abcdef0ull, 0xfedcba9876543210ull
  };
  for (unsigned a = 0; a != 8; ++a)
    for (unsigned b = 0; b != 8; ++b)
      for (unsigned count = 0; count != 9; ++count) {
        U64 expected = high_product(inputs[a], inputs[b]);
        for (unsigned i = 0; i != count; ++i)
          expected = expected * 33 + bytes[i];
        expected = (expected ^ state[0]) + state[1];
        expected = (expected ^ state[2]) + state[3];
        expected = (expected ^ state[4]) + state[5];
        expected = (expected ^ state[6]) + state[7];
        if (integer_path(inputs[a], inputs[b], state, bytes, count) != expected)
          return 1;
      }
  // All are exactly representable as double; no out-of-range conversion.
  for (unsigned i = 0; i != 8; ++i) {
    U128 value = (static_cast<U128>(1) << 80) + (static_cast<U128>(i) << 40);
    U64 expected = static_cast<U64>(value) ^ static_cast<U64>(value >> 64);
    expected = ((expected ^ state[0]) + state[1]) ^ (state[2] + state[3]);
    if (floating_path(value, state) != expected)
      return 2;
  }
  const long long signed_values[] = {0, 1, -1, -9223372036854775807ll - 1};
  for (unsigned i = 0; i != 4; ++i) {
    U64 upper = signed_values[i] < 0 ? ~0ull : 0;
    U64 expected = upper ^ static_cast<U64>(signed_values[i]) ^ state[0];
    if (signed_path(signed_values[i], state) != expected)
      return 3;
  }
  return 0;
}
