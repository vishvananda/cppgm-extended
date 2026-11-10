// Fixed copies and zeroing check payloads, canaries and values live across copies.
using U = unsigned long;
template<unsigned N> struct Blob { unsigned char bytes[N]; };
template<unsigned N> __attribute__((noinline)) Blob<N> copy(Blob<N> const* p, U key) {
  Blob<N> out = *p;
  out.bytes[N / 2] ^= static_cast<unsigned char>(key);
  return out;
}
template<unsigned N> __attribute__((noinline)) Blob<N> zero(U key) {
  Blob<N> out = {};
  out.bytes[N / 3] = static_cast<unsigned char>(key);
  return out;
}
template<unsigned N> __attribute__((noinline)) U checksum(Blob<N> const* p) {
  U h = 0;
  for (unsigned j = 0; j != N; ++j) h = h * 31 + p->bytes[j];
  return h;
}
template<unsigned N> __attribute__((noinline)) U pressure(Blob<N> const* p, U x) {
  U a = x + p->bytes[0], b = x ^ p->bytes[1], c = x * 3 + p->bytes[2];
  U d = x * 5 + p->bytes[3], e = x ^ p->bytes[4], f = x * 7 + p->bytes[5];
  U g = x * 11 + p->bytes[6], h = x ^ p->bytes[7];
  Blob<N> local = *p;
  local.bytes[N - 1] ^= static_cast<unsigned char>(x);
  return checksum(&local) ^ (a + b * 3 + c * 5 + d * 7 + e * 11 + f * 13 + g * 17 + h * 19);
}
template<unsigned N> U test(U key) {
  Blob<N> input;
  for (unsigned j = 0; j != N; ++j) input.bytes[j] = static_cast<unsigned char>((key >> (j & 7)) ^ j);
  struct Guarded { U lo; Blob<N> value; U hi; } result = {key, {}, ~key};
  result.value = copy(&input, key);
  if (result.lo != key || result.hi != ~key) return 1;
  U h = checksum(&result.value);
  for (unsigned j = 0; j != N; ++j) {
    unsigned char expected = input.bytes[j];
    if (j == N / 2) expected ^= static_cast<unsigned char>(key);
    if (result.value.bytes[j] != expected) return 2;
  }
  result.value = zero<N>(key);
  if (result.lo != key || result.hi != ~key) return 3;
  for (unsigned j = 0; j != N; ++j)
    if (result.value.bytes[j] != (j == N / 3 ? static_cast<unsigned char>(key) : 0)) return 4;
  return h ^ checksum(&result.value) ^ pressure(&input, key);
}
int main() {
  U seed = 1, h = 0;
  for (unsigned i = 0; i != 1024; ++i) {
    seed = seed * 6364136223846793005UL + 1;
    h = h * 31 + test<16>(seed);
    h ^= test<32>(seed);
    h += test<64>(seed);
    h ^= test<127>(seed);
    h += test<128>(seed);
    h ^= test<136>(seed);
    h += test<138>(seed);
    h ^= test<192>(seed);
    h += test<255>(seed);
    h ^= test<256>(seed);
    h += test<257>(seed);
    h ^= test<512>(seed);
  }
  return h != 1460851814927019824UL;
}
