extern "C" {
__attribute__((noinline)) unsigned long integer_truth(int x) {
  return static_cast<unsigned char>(x != 0);
}
__attribute__((noinline)) unsigned long floating_truth(double x) {
  return static_cast<unsigned char>(x != 0.0);
}
__attribute__((noinline)) unsigned long unsigned_byte(unsigned long x) {
  return static_cast<unsigned char>(x);
}
__attribute__((noinline)) long signed_byte(long x) {
  return static_cast<signed char>(x);
}
}
int main() {
  for (int i = 0; i != 1024; ++i) {
    if (integer_truth(i) != static_cast<unsigned long>(i != 0)) return 1;
    if (unsigned_byte(i) != static_cast<unsigned>(i & 255)) return 2;
    int low = i & 255;
    long expected = low < 128 ? low : low - 256;
    if (signed_byte(i) != expected) return 3;
  }
  const double inputs[] = {0.0, -0.0, 1.5, -2.5, __builtin_nan(""), __builtin_inf()};
  for (unsigned i = 0; i != sizeof(inputs) / sizeof(inputs[0]); ++i)
    if (floating_truth(inputs[i]) != static_cast<unsigned long>(inputs[i] != 0.0)) return 4;
  return 0;
}
