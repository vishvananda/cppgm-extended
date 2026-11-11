using U = unsigned __int128;
using I = __int128;
extern "C" __attribute__((noinline)) U mul(U a, U b) { return a * b; }
extern "C" __attribute__((noinline)) U divu(U a, U b) { return a / b; }
extern "C" __attribute__((noinline)) U modu(U a, U b) { return a % b; }
extern "C" __attribute__((noinline)) I divs(I a, I b) { return a / b; }
extern "C" __attribute__((noinline)) I mods(I a, I b) { return a % b; }
extern "C" __attribute__((noinline)) float to_float(I a) { return a; }
extern "C" __attribute__((noinline)) double to_double(U a) { return a; }
extern "C" __attribute__((noinline)) I from_double(double a) { return a; }
extern "C" __attribute__((noinline)) U from_float(float a) { return a; }
extern "C" __attribute__((noinline)) long double to_quad(U a) { return a; }
extern "C" __attribute__((noinline)) U from_quad(long double a) { return a; }
extern "C" __attribute__((noinline)) float unsigned_float(U a) { return a; }
extern "C" __attribute__((noinline)) double signed_double(I a) { return a; }
extern "C" __attribute__((noinline)) I signed_float(float a) { return a; }
extern "C" __attribute__((noinline)) U unsigned_double(double a) { return a; }
extern "C" int observe() {
 U a = (U(0x1122334455667788ULL) << 64) | 0xfedcba9876543210ULL;
 U d = (U(1) << 67) + 29;
 U q = divu(a, d), r = modu(a, d);
 if (q * d + r != a || r >= d || mul(a, 1) != a || mul(a, 0) != 0) return 1;
 I negative = -I(a);
 if (divs(negative, I(d)) != -I(q) || mods(negative, I(d)) != -I(r)) return 2;
 if (divs(negative, -I(d)) != I(q) || mods(negative, -I(d)) != -I(r)) return 3;
 if (mul((U(1) << 64) + 1, (U(1) << 64) + 1) != (U(1) << 65) + 1) return 4;
 I minimum = I(U(1) << 127);
 if (to_float(minimum) != -0x1p127f || to_double(U(-1)) != 0x1p128) return 5;
 if (from_double(-0x1p100) != -I(U(1) << 100) || from_float(0x1p100f) != U(1) << 100) return 6;
 if (from_double(-42.75) != -42 || from_float(42.75f) != 42) return 7;
 U exact = (U(1) << 100) + 17;
 if (from_quad(to_quad(exact)) != exact) return 8;
 if (unsigned_float(U(1) << 100) != 0x1p100f || signed_double(minimum) != -0x1p127) return 9;
 if (signed_float(-0x1p100f) != -I(U(1) << 100) || unsigned_double(0x1p100) != U(1) << 100) return 10;
 return 17;
}
