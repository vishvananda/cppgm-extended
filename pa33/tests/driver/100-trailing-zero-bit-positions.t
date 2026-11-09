__attribute__((noinline)) unsigned low_bit(unsigned value) {
  return __builtin_ctz(value);
}
__attribute__((noinline)) unsigned low_bit_wide(unsigned long long value) {
  return __builtin_ctzll(value);
}
__attribute__((noinline)) unsigned first_bit(unsigned long long value) {
  unsigned count = 0;
  while ((value & 1) == 0) { value >>= 1; ++count; }
  return count;
}
int main() {
  for (unsigned i = 0; i != 64; ++i) {
    unsigned long long one = 1ULL << i;
    unsigned long long many = (~0ULL << i) | one;
    if (low_bit_wide(one) != i || low_bit_wide(many) != first_bit(many)) return 1;
    if ((low_bit_wide(one) == 0) != (i == 0)) return 2;
    if (i < 32 && (low_bit(static_cast<unsigned>(one)) != i ||
                   low_bit(static_cast<unsigned>(many)) != i)) return 3;
  }
  return 0;
}
