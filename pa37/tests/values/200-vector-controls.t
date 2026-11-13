#include "200-vector-controls.h"
template<class V, unsigned N> bool same(V a, V b) {
 for (unsigned i = 0; i != N; ++i) if (a[i] != b[i]) return false;
 return true;
}
#ifdef __CPPGM__
template<unsigned Control> int check_control(int seed) {
#else
int check_control(int seed, unsigned Control) {
#endif
  Quads a = {static_cast<long long>(0xfedcba9876543210ULL ^ seed), static_cast<long long>(0x89abcdef01234567ULL)};
  Quads b = {seed ? -1LL : 0, static_cast<long long>(0xa5a5a5a5a5a5a5a5ULL)};
  if (!same<Quads,2>(__builtin_ia32_pclmulqdq128(a,b,Control),expected_carryless(a,b,Control))) return 1;
  Singles x = {seed + 0.5f, -2.5f, 3.5f, -4.75f};
  Singles y = {2.0f, 3.0f, -1.0f, 0.5f};
  Doubles p = {seed + 0.5, -2.5}, q = {3.5, -0.75};
  if (!same<Singles,4>(__builtin_ia32_dpps(x,y,Control),expected_dot_s(x,y,Control))) return 2;
  if (!same<Doubles,2>(__builtin_ia32_dppd(p,q,Control),expected_dot_d(p,q,Control))) return 3;
  if (!same<Singles,4>(__builtin_ia32_insertps128(x,y,Control),expected_insert(x,y,Control))) return 4;
  if (!same<Singles,4>(__builtin_ia32_roundps(x,Control),expected_round_s(x,Control))) return 5;
  if (!same<Doubles,2>(__builtin_ia32_roundpd(p,Control),expected_round_d(p,Control))) return 6;
  if (!same<Singles,4>(__builtin_ia32_roundss(x,y,Control),expected_round_ss(x,y,Control))) return 7;
  if (!same<Doubles,2>(__builtin_ia32_roundsd(p,q,Control),expected_round_sd(p,q,Control))) return 8;
  return 0;
}
#ifdef __CPPGM__
template<unsigned Control> struct Check {
 static int run(int seed) {
  int result = check_control<Control>(seed);
  return result ? result : Check<Control-1>::run(seed);
 }
};
template<> struct Check<0> { static int run(int seed) { return check_control<0>(seed); } };
#endif
extern "C" int observe(int seed) {
 Words a = {-1, 7, 2, 2, 5, 8, 9, 2};
 a[seed & 7] = -2;
 if (!same<Words,8>(__builtin_ia32_phminposuw128(a),expected_minpos(a))) return 9;
#ifdef __CPPGM__
 int result = Check<255>::run(seed);
 return result ? result : 17;
#else
 for (unsigned control = 0; control != 256; ++control) {
  int result = check_control(seed, control);
  if (result) return result;
 }
 return 17;
#endif
}
