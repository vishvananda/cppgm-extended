extern "C" unsigned long long wide_local(unsigned long long low, unsigned long long high) {
  unsigned __int128 x = (static_cast<unsigned __int128>(high) << 64) | low;
  unsigned __int128 y = (static_cast<unsigned __int128>(0x123456789abcdef0ULL) << 64) | 0xfedcba9876543210ULL;
  unsigned __int128 result = ((x + y) ^ (x - y)) >> 64;
  return static_cast<unsigned long long>(result);
}
