using U = unsigned __int128;
using I = __int128;
volatile unsigned long long input_low = 0xfedcba9876543210ULL;
volatile unsigned long long input_high = 0x123456789abcdef0ULL;
extern "C" __attribute__((noinline)) U add(U a, U b) { return a + b; }
extern "C" __attribute__((noinline)) U sub(U a, U b) { return a - b; }
extern "C" __attribute__((noinline)) U left(U a, unsigned b) { return a << b; }
extern "C" __attribute__((noinline)) U right(U a, unsigned b) { return a >> b; }
extern "C" __attribute__((noinline)) I signed_right(I a, unsigned b) { return a >> b; }
extern "C" int observe() {
 U a = (U(input_high) << 64) | input_low;
 U b = (U(0xededededededededULL) << 64) | 0xaaaaaaaaaaaaaaaaULL;
 if (sub(add(a, b), b) != a || add(~U(0), 1) != 0 || sub(0, 1) != ~U(0)) return 1;
 if ((a & b) != ((a | b) ^ (a ^ b))) return 2;
 if (-I(a) != I(~a + 1) || !a || !!U(0)) return 3;
 if (I(-7) >= I(9) || I(-7) <= I(-8) || U(-7) < U(9)) return 4;
 if (!(a < b) || !(a <= a) || !(b >= a) || a == b || !(a != b)) return 5;
 if (I(static_cast<signed char>(-3)) != -3 || U(static_cast<unsigned char>(253)) != 253) return 6;
 if (static_cast<unsigned long long>(a) != input_low || static_cast<unsigned>(a) != 0x76543210u) return 7;
 const unsigned shifts[] = {0, 1, 31, 32, 63, 64, 65, 95, 96, 127};
 for (unsigned j = 0; j < 10; ++j) {
  unsigned n = shifts[j];
  U l = left(a, n), r = right(a, n);
  unsigned long long lo = input_low, hi = input_high;
  unsigned long long ll = n < 64 ? lo << n : 0;
  unsigned long long lh = n == 0 ? hi : n < 64 ? (hi << n) | (lo >> (64 - n)) : lo << (n - 64);
  unsigned long long rl = n == 0 ? lo : n < 64 ? (lo >> n) | (hi << (64 - n)) : hi >> (n - 64);
  unsigned long long rh = n < 64 ? hi >> n : 0;
  if (static_cast<unsigned long long>(l) != ll || static_cast<unsigned long long>(l >> 64) != lh) return 8;
  if (static_cast<unsigned long long>(r) != rl || static_cast<unsigned long long>(r >> 64) != rh) return 9;
  I negative = -I(a);
  if (signed_right(negative, n) != -I(right(a - 1, n)) - 1) return 10;
 }
 volatile U stored = a;
 U snapshot = stored;
 stored = b;
 if (snapshot != a || stored != b) return 11;
 int array[2] = {7, 19};
 volatile I index = 1;
 if (array[index] != 19) return 12;
 return 17;
}
