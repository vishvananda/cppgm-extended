using Words = short __attribute__((vector_size(16)));
using Quads = long long __attribute__((vector_size(16)));
using Singles = float __attribute__((vector_size(16)));
using Doubles = double __attribute__((vector_size(16)));
#ifndef __CPPGM__
#define __builtin_ia32_phminposuw128 expected_minpos
#define __builtin_ia32_pclmulqdq128 expected_carryless
#define __builtin_ia32_dpps expected_dot_s
#define __builtin_ia32_dppd expected_dot_d
#define __builtin_ia32_insertps128 expected_insert
#define __builtin_ia32_roundps expected_round_s
#define __builtin_ia32_roundpd expected_round_d
#define __builtin_ia32_roundss expected_round_ss
#define __builtin_ia32_roundsd expected_round_sd
#endif
extern "C" Words expected_minpos(Words);
extern "C" Quads expected_carryless(Quads, Quads, int);
extern "C" Singles expected_dot_s(Singles, Singles, int);
extern "C" Doubles expected_dot_d(Doubles, Doubles, int);
extern "C" Singles expected_insert(Singles, Singles, int);
extern "C" Singles expected_round_s(Singles, int);
extern "C" Doubles expected_round_d(Doubles, int);
extern "C" Singles expected_round_ss(Singles, Singles, int);
extern "C" Doubles expected_round_sd(Doubles, Doubles, int);
