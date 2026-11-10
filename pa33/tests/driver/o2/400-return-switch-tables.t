extern "C" {
__attribute__((noinline)) int scalar(int x) {
  switch (x) {
    case -3: return -11;
    case -2: return 25;
    case 0: return 70;
    case 2: return -128;
    case 3: return 127;
    default: return 19;
  }
}
__attribute__((noinline)) unsigned char narrow(int x) {
  switch (x) {
    case 0: return 255;
    case 1: return 128;
    case 2: return 17;
    case 3: return 200;
    default: return 99;
  }
}
__attribute__((noinline)) bool truth(int x) {
  switch (x) {
    case 0: return false;
    case 1: return true;
    case 2: return true;
    case 3: return false;
    default: return true;
  }
}
__attribute__((noinline)) unsigned long long wide(unsigned long long x) {
  switch (x) {
    case 0xfffffffffffffff0ULL: return 0xffffffffffffffffULL;
    case 0xfffffffffffffff1ULL: return 0x8000000000000000ULL;
    case 0xfffffffffffffff2ULL: return 0x7fffffffffffffffULL;
    case 0xfffffffffffffff3ULL: return 0;
    default: return 123;
  }
}
__attribute__((noinline)) char *adjust(char *p, unsigned x) {
  switch (x) {
    case 0: return p - 40;
    case 1: return p - 72;
    case 3: return p + 16;
    case 4: return p - 48;
    case 7: return p + 8;
    default: return p - 32;
  }
}
__attribute__((noinline)) char *nullable(char *p, unsigned x) {
  switch (x) {
    case 0: return p ? p - 40 : 0;
    case 1: return p ? p - 72 : 0;
    case 2: return p ? p + 16 : 0;
    case 3: return p ? p - 48 : 0;
    default: return p;
  }
}
__attribute__((noinline)) char *different(char *p, char *q, unsigned x) {
  switch (x) {
    case 0: return p - 40;
    case 1: return q - 72;
    case 2: return p + 16;
    case 3: return q - 48;
    default: return p;
  }
}
__attribute__((noinline)) char *from_word(unsigned long bits, unsigned x) {
  switch (x) {
    case 0: return reinterpret_cast<char *>(bits) - 40;
    case 1: return reinterpret_cast<char *>(bits) - 72;
    case 2: return reinterpret_cast<char *>(bits) + 16;
    case 3: return reinterpret_cast<char *>(bits) - 48;
    default: return reinterpret_cast<char *>(bits);
  }
}
int effects;
__attribute__((noinline)) int holes(int x) {
  switch (x) {
    case 0: return 31;
    case 2: return 37;
    case 4: return 41;
    case 6: return 43;
    default: ++effects; return 47;
  }
}
__attribute__((noinline)) int dense_default(int x) {
  switch (x) {
    case 0: return 31;
    case 1: return 37;
    case 2: return 41;
    case 3: return 43;
    default: ++effects; return 47;
  }
}
__attribute__((noinline)) int side_case(int x) {
  switch (x) {
    case 0: ++effects; return 31;
    case 1: return 37;
    case 2: return 41;
    case 3: return 43;
    default: return 47;
  }
}
}
int main() {
  const int scalars[] = {-11, 25, 19, 70, 19, -128, 127};
  const unsigned char bytes_expected[] = {255, 128, 17, 200};
  char storage[512] = {};
  char *p = storage + 160;
  char *q = storage + 400;
  for (int x = -128; x != 256; ++x) {
    if (scalar(x) != (x >= -3 && x <= 3 ? scalars[x + 3] : 19)) return 1;
    if (narrow(x) != (x >= 0 && x < 4 ? bytes_expected[x] : 99)) return 2;
    if (truth(x) != (x != 0 && x != 3)) return 3;
  }
  const int offsets[] = {-40, -72, -32, 16, -48, -32, -32, 8};
  for (unsigned x = 0; x != 32; ++x) {
    if (adjust(p, x) != p + (x < 8 ? offsets[x] : -32)) return 4;
    if (nullable(0, x) != 0) return 5;
    char *expected = x == 0 ? p - 40 : x == 1 ? q - 72 : x == 2 ? p + 16 : x == 3 ? q - 48 : p;
    if (different(p, q, x) != expected) return 6;
    char *word_expected = x == 0 ? p - 40 : x == 1 ? p - 72 : x == 2 ? p + 16 : x == 3 ? p - 48 : p;
    if (from_word(reinterpret_cast<unsigned long>(p), x) != word_expected) return 14;
  }
  if (wide(0xfffffffffffffff0ULL) != 0xffffffffffffffffULL) return 7;
  if (wide(0xfffffffffffffff1ULL) != 0x8000000000000000ULL) return 8;
  if (wide(0xfffffffffffffff2ULL) != 0x7fffffffffffffffULL || wide(0xfffffffffffffff3ULL) != 0) return 9;
  if (wide(0) != 123 || wide(0xffffffffffffffffULL) != 123) return 10;
  effects = 0;
  if (holes(1) != 47 || holes(3) != 47 || holes(0) != 31 || effects != 2) return 11;
  if (dense_default(-1) != 47 || dense_default(4) != 47 || dense_default(2) != 41 || effects != 4) return 12;
  if (side_case(0) != 31 || side_case(1) != 37 || effects != 5) return 13;
  return 0;
}
