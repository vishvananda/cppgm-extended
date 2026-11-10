// Several retained values compete with a loop pointer and accumulator.
__attribute__((noinline)) unsigned long pressure(
    volatile unsigned long* in, const unsigned char* text,
    unsigned length, unsigned mode)
{
  unsigned long a0 = in[0], a1 = in[1], a2 = in[2], a3 = in[3];
  unsigned long a4 = in[4], a5 = in[5], a6 = in[6], a7 = in[7];
  unsigned long result = 0;
  switch (mode) {
  case 0: result = a0 ^ (a1 * 3ul); break;
  case 1: result = (a1 ^ (a2 * 5ul)) + 1; break;
  case 2: result = (a2 ^ (a3 * 7ul)) + 2; break;
  case 3: result = (a3 ^ (a4 * 9ul)) + 3; break;
  default: {
    const unsigned char* p = text;
    const unsigned char* end = text + length;
    while (p != end) {
      unsigned char digit = static_cast<unsigned char>(*p - 48);
      if (digit > 9)
        break;
      result = result * 10 + digit;
      ++p;
    }
    result ^= static_cast<unsigned long>(p - text);
    break;
  }
  }
  result = (result ^ (a0 + (result << 1))) * 3ul;
  result = (result ^ (a1 + (result << 2))) * 5ul;
  result = (result ^ (a2 + (result << 3))) * 7ul;
  result = (result ^ (a3 + (result << 4))) * 9ul;
  result = (result ^ (a4 + (result << 5))) * 11ul;
  result = (result ^ (a5 + (result << 1))) * 13ul;
  result = (result ^ (a6 + (result << 2))) * 15ul;
  result = (result ^ (a7 + (result << 3))) * 17ul;
  return result;
}

int main()
{
  volatile unsigned long inputs[2][8] = {
    {1, 3, 5, 7, 11, 13, 17, 19},
    {~0ul, 1ul << 63, 0, 0x1020304050607080ul,
     0xfedcba9876543210ul, 123456789ul, 0x8000000000000001ul, 42}
  };
  const unsigned char text[] = "12345678901234567890x9";
  const unsigned modes[] = {4, 4, 4, 4, 0, 1, 2, 3};
  const unsigned starts[] = {0, 0, 20, 0, 0, 0, 0, 0};
  const unsigned lengths[] = {0, 22, 2, 3, 22, 22, 22, 22};
  // Independently evaluated modulo 2^64, including unsigned overflow.
  const unsigned long expected[2][8] = {
    {0x23fd8b8ffe1aul, 0x465d53be0385fabcul, 0x23fd8b8ffe1aul, 0x11557c596bef02ul,
     0x63d24ca950412ul, 0x7ca6e343dd435ul, 0x1362b7c398bf0cul, 0xf132b637c33c1ul},
    {0x17a3a4ba34a68fa3ul, 0x6b560f81cae44cbdul, 0x17a3a4ba34a68fa3ul, 0x17b498f08766224bul,
     0x978f13e201f99198ul, 0x978eb4673b447b16ul, 0x9a4d6763939a2ea1ul, 0xea344ad196a6a86cul}
  };
  for (unsigned set = 0; set != 2; ++set)
    for (unsigned test = 0; test != 8; ++test)
      if (pressure(inputs[set], text + starts[test], lengths[test], modes[test])
          != expected[set][test])
        return 1;
  return 0;
}
