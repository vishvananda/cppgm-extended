struct Pad { long payload; };
struct Base { unsigned kind; };
struct Derived : Pad, Base { long value; };
__attribute__((noinline)) Derived* down(Base* p) {
  return static_cast<Derived*>(p);
}
__attribute__((noinline)) long chosen(long x, long other) {
  return x == 37 ? 37 : other;
}
__attribute__((noinline)) long not_chosen(long x, long other) {
  return x != 37 ? other : 37;
}
__attribute__((noinline)) long unsigned_bit(unsigned long long x) {
  return ((x >> 2) & 1) ? 11 : 29;
}
__attribute__((noinline)) long signed_bit(long long x) {
  return ((x >> 2) & 1) ? 11 : 29;
}
__attribute__((noinline)) void change(long* p) { *p = 9; }
__attribute__((noinline)) long before_change(long* p) {
  long old = *p;
  bool same = old == 5;
  change(p);
  return same ? 11 : 29;
}
__attribute__((noinline)) double positive_zero(double x) {
  return x == 0.0 ? 0.0 : x;
}
unsigned long long bits(double x) {
  unsigned long long result;
  __builtin_memcpy(&result, &x, 8);
  return result;
}
int main() {
  Derived d;
  Base* p = &d;
  if (down(p) != &d || down(nullptr) != nullptr) return 1;
  unsigned long long random = 0x123456789abcdefULL;
  for (unsigned i = 0; i != 1000; ++i) {
    random = random * 6364136223846793005ULL + 1442695040888963407ULL;
    long expected = (random & 4) ? 11 : 29;
    if (unsigned_bit(random) != expected ||
        signed_bit(static_cast<long long>(random)) != expected) return 2;
    long x = static_cast<long>(i % 75);
    long other = static_cast<long>(random);
    long selected = x == 37 ? 37 : other;
    if (chosen(x, other) != selected || not_chosen(x, other) != selected)
      return 3;
  }
  long value = 5;
  if (before_change(&value) != 11 || value != 9) return 4;
  value = 0;
  if (before_change(&value) != 29 || value != 9) return 5;
  if (bits(positive_zero(-0.0)) != 0 || bits(positive_zero(0.0)) != 0)
    return 6;
  return 0;
}
