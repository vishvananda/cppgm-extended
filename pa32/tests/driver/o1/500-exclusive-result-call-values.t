struct Result { long a, b, c, d; };
struct VolatileResult { volatile long a; long b, c, d; };
extern "C" {
__attribute__((noinline, pure)) long peek(const long* p) { return *p; }
__attribute__((noinline)) Result stored(const long* p) {
  Result out = {};
  out.a = peek(p);
  out.b = peek(p);
  return out;
}
__attribute__((noinline)) Result copied(const long* p) {
  const Result seed = {11, 13, 17, 19};
  long first = peek(p);
  Result out = seed;
  out.a = first;
  out.b = peek(p);
  return out;
}
__attribute__((noinline)) Result zeroed(const long* p) {
  long first = peek(p);
  Result out = {};
  out.a = first;
  out.b = peek(p);
  return out;
}
__attribute__((noinline)) Result escaped() {
  Result out = {};
  out.a = 2;
  long first = peek(&out.a);
  out.a = 7;
  out.b = first + peek(&out.a);
  return out;
}
__attribute__((noinline)) VolatileResult observed(const long* p) {
  VolatileResult out = {};
  out.a = peek(p);
  out.b = peek(p);
  return out;
}
}
int main() {
  for (long i = -100; i != 101; ++i) {
    long value = i * 0x100000001L - 512;
    Result a = stored(&value), b = copied(&value), c = zeroed(&value), d = escaped();
    VolatileResult e = observed(&value);
    if (a.a != value || a.b != value || a.c != 0 || a.d != 0) return 1;
    if (b.a != value || b.b != value || b.c != 17 || b.d != 19) return 2;
    if (c.a != value || c.b != value || c.c != 0 || c.d != 0) return 3;
    if (d.a != 7 || d.b != 9 || d.c != 0 || d.d != 0) return 4;
    if (e.a != value || e.b != value || e.c != 0 || e.d != 0) return 5;
  }
  return 0;
}
