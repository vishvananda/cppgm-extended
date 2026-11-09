extern "C" {
__attribute__((pure, noinline)) int read_value(const int* p) noexcept { return *p; }
__attribute__((const, noinline)) unsigned calculate(unsigned x) noexcept {
  return (x * 1664525u + 1013904223u) ^ (x >> 7);
}
__attribute__((noinline)) int repeat_read(const int* p) {
  return read_value(p) + read_value(p);
}
__attribute__((noinline)) int write_between(int* p) {
  int a = read_value(p);
  *p = 7;
  return a + read_value(p);
}
__attribute__((noinline)) unsigned constant_across_store(unsigned x, unsigned* p) {
  unsigned a = calculate(x);
  *p = 37;
  return a + calculate(x);
}
__attribute__((noinline)) int ordinary(const volatile int* p) { return *p; }
__attribute__((noinline)) int ordinary_twice(const volatile int* p) {
  return ordinary(p) + ordinary(p);
}
__attribute__((noinline)) int volatile_between(const int* p, volatile int* output) {
  int a = read_value(p);
  *output = 11;
  return a + read_value(p);
}
struct Payload { long words[4]; };
__attribute__((const, noinline)) long payload_value(Payload value) noexcept {
  return value.words[0] + value.words[3];
}
__attribute__((noinline)) long payload_between(Payload* p) {
  long a = payload_value(*p);
  p->words[3] = 7;
  return a + payload_value(*p);
}
__attribute__((noinline)) int can_throw(const int* p) {
  if (!p) throw 19;
  return *p;
}
__attribute__((noinline)) int repeat_throw(const int* p) {
  return can_throw(p) + can_throw(p);
}
}
int main() {
  for (unsigned i = 0; i != 1024; ++i) {
    int value = static_cast<int>(i) - 512;
    if (repeat_read(&value) != value * 2) return 1;
    int expected = value + 7;
    if (write_between(&value) != expected || value != 7) return 2;
    unsigned stored = 0;
    unsigned result = (i * 1664525u + 1013904223u) ^ (i >> 7);
    if (constant_across_store(i, &stored) != result + result || stored != 37) return 3;
    if (ordinary_twice(&value) != 14) return 4;
    if (volatile_between(&value, &value) != 18 || value != 11) return 5;
    Payload payload = {{1, 2, 3, 4}};
    if (payload_between(&payload) != 13 || payload.words[3] != 7) return 6;
    if (repeat_throw(&value) != 22) return 7;
  }
  try { if (repeat_throw(nullptr) != 0) return 8; return 10; }
  catch (int value) { if (value != 19) return 9; }
  return 0;
}
