extern "C" unsigned update_word(const unsigned char* data, unsigned long length, unsigned value)
{
  for (unsigned long i = 0; i < length; ++i) {
    value ^= data[i];
    for (unsigned bit = 0; bit < 8; ++bit)
      value = (value & 1) ? (value >> 1) ^ 0x82f63b78u : value >> 1;
  }
  return value;
}

extern "C" unsigned update_nibble(unsigned value)
{
  for (unsigned bit = 0; bit < 4; ++bit)
    value = (value & 1) ? (value >> 1) ^ 0xa7b94121u : value >> 1;
  return value;
}

__attribute__((noinline)) unsigned oracle(unsigned value, unsigned polynomial, unsigned steps)
{
  for (unsigned bit = 0; bit < steps; ++bit)
    value = (value >> 1) ^ ((0u - (value & 1)) & polynomial);
  return value;
}

volatile unsigned seed = 0x12345678u;

int main()
{
  unsigned char data[65];
  unsigned state = seed;
  for (unsigned i = 0; i < 65; ++i) {
    state = state * 1664525u + 1013904223u;
    data[i] = static_cast<unsigned char>(state >> 24);
  }
  for (unsigned length = 0; length <= 65; ++length) {
    state = state * 1664525u + 1013904223u;
    unsigned expected = state;
    for (unsigned i = 0; i < length; ++i)
      expected = oracle(expected ^ data[i], 0x82f63b78u, 8);
    if (update_word(data, length, state) != expected) return 1;
    if (update_nibble(state) != oracle(state, 0xa7b94121u, 4)) return 2;
  }
  return 0;
}
