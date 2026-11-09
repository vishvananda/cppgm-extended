extern "C" {
__attribute__((noinline)) unsigned char narrow(unsigned x) {
  return static_cast<unsigned long>(x);
}
__attribute__((noinline)) int sign_extend(signed char x) {
  return static_cast<long>(x);
}
__attribute__((noinline)) unsigned zero_extend(unsigned char x) {
  return static_cast<unsigned long>(x);
}
__attribute__((noinline)) unsigned retype(int x) {
  return static_cast<long>(x);
}
__attribute__((noinline)) double floating_guard(double x) {
  return static_cast<float>(x);
}
}
int main() {
  for (int i = 0; i != 1024; ++i) {
    unsigned low = static_cast<unsigned>(i & 255);
    int signed_low = low < 128 ? static_cast<int>(low) : static_cast<int>(low) - 256;
    unsigned pattern = (static_cast<unsigned>(i) << 24) | static_cast<unsigned>(i);
    if (narrow(pattern) != low) return 1;
    if (sign_extend(static_cast<signed char>(i)) != signed_low) return 2;
    if (zero_extend(static_cast<unsigned char>(i)) != low) return 3;
    if (retype(i - 512) != static_cast<unsigned>(i - 512)) return 4;
  }
  const double values[] = {1.0000000009313226, -1.0000000009313226, 16777217.0, 0.1, -0.1, -0.0};
  for (unsigned i = 0; i != sizeof(values) / sizeof(values[0]); ++i)
    if (floating_guard(values[i]) != static_cast<double>(static_cast<float>(values[i]))) return 5;
  return 0;
}
