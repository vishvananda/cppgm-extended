#include "200-vector-controls.h"
extern "C" Words expected_minpos(Words a) {
 unsigned short smallest = static_cast<unsigned short>(a[0]);
 unsigned index = 0;
 for (unsigned i = 1; i != 8; ++i)
  if (static_cast<unsigned short>(a[i]) < smallest) { smallest = a[i]; index = i; }
 Words result = {0,0,0,0,0,0,0,0};
 result[0] = static_cast<short>(smallest); result[1] = static_cast<short>(index);
 return result;
}
extern "C" Quads expected_carryless(Quads a, Quads b, int control) {
 unsigned __int128 multiplicand = static_cast<unsigned long long>(a[control & 1]);
 unsigned long long multiplier = static_cast<unsigned long long>(b[(control >> 4) & 1]);
 unsigned __int128 result = 0;
 for (unsigned i = 0; i != 64; ++i) {
  if ((multiplier & 1) != 0) result ^= multiplicand;
  multiplicand <<= 1; multiplier >>= 1;
 }
 return (Quads){static_cast<long long>(result), static_cast<long long>(result >> 64)};
}
extern "C" Singles expected_dot_s(Singles a, Singles b, int control) {
 float p[4] = {};
 for (int i = 0; i != 4; ++i) if (control & (16 << i)) p[i] = a[i] * b[i];
 float sum = (p[0] + p[1]) + (p[2] + p[3]);
 Singles result = {0,0,0,0};
 for (int i = 0; i != 4; ++i) if (control & (1 << i)) result[i] = sum;
 return result;
}
extern "C" Doubles expected_dot_d(Doubles a, Doubles b, int control) {
 double p0 = (control & 16) ? a[0] * b[0] : 0;
 double p1 = (control & 32) ? a[1] * b[1] : 0;
 double sum = p0 + p1;
 return (Doubles){(control & 1) ? sum : 0, (control & 2) ? sum : 0};
}
extern "C" Singles expected_insert(Singles a, Singles b, int control) {
 a[(control >> 4) & 3] = b[(control >> 6) & 3];
 for (int i = 0; i != 4; ++i) if (control & (1 << i)) a[i] = 0;
 return a;
}
template<class T> T rounded(T value, int control) {
 int direction = (control & 4) ? 0 : control & 3;
 if (direction == 1) return __builtin_floor(value);
 if (direction == 2) return __builtin_ceil(value);
 if (direction == 3) return __builtin_trunc(value);
 T down = __builtin_floor(value);
 T fraction = value - down;
 if (fraction < T(0.5)) return down;
 if (fraction > T(0.5)) return down + 1;
 return (static_cast<long long>(down) & 1) == 0 ? down : down + 1;
}
extern "C" Singles expected_round_s(Singles a, int control) {
 for (int i = 0; i != 4; ++i) a[i] = rounded(a[i], control);
 return a;
}
extern "C" Doubles expected_round_d(Doubles a, int control) {
 for (int i = 0; i != 2; ++i) a[i] = rounded(a[i], control);
 return a;
}
extern "C" Singles expected_round_ss(Singles a, Singles b, int control) {
 a[0] = rounded(b[0], control); return a;
}
extern "C" Doubles expected_round_sd(Doubles a, Doubles b, int control) {
 a[0] = rounded(b[0], control); return a;
}
