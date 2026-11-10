typedef long long Pair __attribute__((vector_size(16)));
__attribute__((target("pclmul"))) Pair product(Pair a, Pair b, int selector) {
  return __builtin_ia32_pclmulqdq128(a, b, selector);
}
