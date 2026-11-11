using U = unsigned __int128;
using I = __int128;
using Word = unsigned long long;
extern "C" __attribute__((noinline)) U product(U a, U b) { return a * b; }
extern "C" __attribute__((noinline)) U quotient(U a, U b) { return a / b; }
extern "C" __attribute__((noinline)) U wide_remainder(U a, U b) { return a % b; }
extern "C" __attribute__((noinline)) I signed_quotient(I a, I b) { return a / b; }
extern "C" __attribute__((noinline)) I signed_remainder(I a, I b) { return a % b; }
Word next(Word& state) {
 state ^= state << 13;
 state ^= state >> 7;
 state ^= state << 17;
 return state;
}
Word mix(Word state, U value) {
 state ^= static_cast<Word>(value);
 state = (state << 9) | (state >> 55);
 return state ^ static_cast<Word>(value >> 64);
}
extern "C" int digest() {
 Word seed = 0x123456789abcdef0ULL, result = 19;
 for (unsigned i = 0; i < 256; ++i) {
  U a = U(next(seed)) << 64;
  a |= next(seed);
  U b = U(next(seed)) << 64;
  b |= next(seed) | 1;
  result = mix(result, a + b);
  result = mix(result, a - b);
  result = mix(result, a ^ b);
  result = mix(result, a & b);
  result = mix(result, product(a, b));
  result = mix(result, quotient(a, b));
  result = mix(result, wide_remainder(a, b));
  U small = (b >> (i % 113)) | 1;
  result = mix(result, quotient(a, small));
  result = mix(result, wide_remainder(a, small));
  I signed_a = -I(a >> 1);
  I signed_b = I(small >> 1) + 1;
  result = mix(result, U(signed_quotient(signed_a, signed_b)));
  result = mix(result, U(signed_remainder(signed_a, signed_b)));
  unsigned shift = i % 128;
  result = mix(result, a << shift);
  result = mix(result, a >> shift);
  result = mix(result, U(signed_a >> shift));
  result ^= (a < b) | ((a >= b) << 2) | ((signed_a < signed_b) << 3);
 }
 return static_cast<unsigned>(result) & 0x7fffffff;
}
