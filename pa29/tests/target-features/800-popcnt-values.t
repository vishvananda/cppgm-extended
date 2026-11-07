volatile unsigned calls;
__attribute__((noinline)) unsigned long long next_value()
{
  ++calls;
  return 0x8000000000000001ull;
}
__attribute__((noinline)) unsigned count64(unsigned long long value)
{
  return __builtin_popcountll(value);
}
__attribute__((noinline)) unsigned count32(unsigned value)
{
  return __builtin_popcount(value);
}
__attribute__((noinline)) unsigned count128(unsigned __int128 value)
{
  return __builtin_popcountg(value);
}
unsigned oracle(unsigned long long value)
{
  unsigned count = 0;
  for (unsigned bit = 0; bit != 64; ++bit)
    count += static_cast<unsigned>((value >> bit) & 1);
  return count;
}
int main()
{
  unsigned long long value = 0;
  for (unsigned n = 0; n != 1000; ++n) {
    if (count64(value) != oracle(value) ||
        count32(static_cast<unsigned>(value)) != oracle(static_cast<unsigned>(value)))
      return 1;
    value = value * 6364136223846793005ull + 1442695040888963407ull;
  }
  for (unsigned bit = 0; bit != 64; ++bit)
    if (count64(1ull << bit) != 1 || count64(~(1ull << bit)) != 63)
      return 2;
  if (count64(0) != 0 || count64(~0ull) != 64 || count32(~0u) != 32)
    return 3;
  unsigned __int128 wide = static_cast<unsigned __int128>(value) << 64;
  if (count128(wide) != oracle(value) || count128(~static_cast<unsigned __int128>(0)) != 128)
    return 4;
  if (__builtin_popcountll(next_value()) != 2 || calls != 1)
    return 5;
  return 0;
}
