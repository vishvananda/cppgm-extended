typedef long long Quad __attribute__((vector_size(32)));
__attribute__((target("avx2"))) Quad halves(Quad a, Quad b) {
  return __builtin_ia32_permti256(a, b, 256);
}
