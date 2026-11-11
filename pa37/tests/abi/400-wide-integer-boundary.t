using U = unsigned __int128;
using I = __int128;
struct Wrapper { U value; };
extern "C" U identity(U);
extern "C" U combine(int, U, unsigned long long, U, int);
extern "C" Wrapper wrapped(Wrapper);
extern "C" U variadic(int, ...);
U (*volatile indirect)(U) = identity;
extern "C" int observe() {
 U a = (U(0x1122334455667788ULL) << 64) | 0xfedcba9876543210ULL;
 U b = (U(0xabcdeULL) << 64) | 123;
 if (indirect(a) != a || identity(U(-1)) != U(-1)) return 1;
 if (combine(-3, a, 0x123456789ULL, b, 7) != (a ^ b) + 0x12345678dULL) return 2;
 if (wrapped(Wrapper{a}).value != a + 1) return 3;
 if (variadic(3, static_cast<unsigned>(11), a, I(-7), 19) != a + 23) return 4;
 if (variadic(3, static_cast<unsigned>(11), U(0), I(-7), 19) != 23) return 5;
 return 17;
}
