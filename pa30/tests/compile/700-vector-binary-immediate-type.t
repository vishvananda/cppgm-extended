typedef long long Pair __attribute__((vector_size(16)));
typedef long long Quad __attribute__((vector_size(32)));
#if !__has_builtin(__builtin_ia32_pclmulqdq128) || !__has_builtin(__builtin_ia32_permti256)
#error required immediate vector operations are not provided
#endif
__attribute__((target("pclmul"))) Pair product(Pair a, Pair b) {
  return __builtin_ia32_pclmulqdq128(a, b, 17);
}
__attribute__((target("avx2"))) Quad halves(Quad a, Quad b) {
  return __builtin_ia32_permti256(a, b, 0x21);
}
