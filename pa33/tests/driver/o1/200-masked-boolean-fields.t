struct Flags {
  unsigned low : 1;
  unsigned middle : 3;
  unsigned unused : 27;
  unsigned high : 1;
};
extern "C" {
__attribute__((noinline)) bool low_bit(const Flags* p) { return p->low; }
__attribute__((noinline)) bool high_bit(const Flags* p) { return p->high; }
__attribute__((noinline)) void copy_low(bool* out, const Flags* p) { *out = p->low; }
__attribute__((noinline)) bool wider_field(const Flags* p) { return p->middle; }
__attribute__((noinline)) bool raw_xor(unsigned x) { return x ^ 1; }
}
int main() {
  for (unsigned i = 0; i != 1024; ++i) {
    Flags value = {i & 1, (i >> 1) & 7, i, (i >> 2) & 1};
    bool copied = false;
    copy_low(&copied, &value);
    if (copied != ((i & 1) != 0) || low_bit(&value) != copied) return 1;
    if (high_bit(&value) != (((i >> 2) & 1) != 0)) return 2;
    if (wider_field(&value) != (((i >> 1) & 7) != 0)) return 3;
    if (raw_xor(i) != ((i ^ 1) != 0)) return 4;
  }
  return 0;
}
