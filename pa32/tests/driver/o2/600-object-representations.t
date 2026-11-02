// Copies preserve padding-visible bytes, floating representations and
// alignment. An escaped self pointer remains a real access to its object.
struct Padded { char tag; int value; };
struct Aligned { alignas(32) unsigned long long values[4]; };
union Bits { double d; unsigned long long u; };
static int order;
int first() { order = order * 10 + 1; return 3; }
int second() { order = order * 10 + 2; return 5; }
int a = first(), b = second();
__attribute__((noinline)) Padded copy(Padded p) { return p; }
__attribute__((noinline)) double carrier(unsigned long long bits) {
    double value; __builtin_memcpy(&value, &bits, sizeof value); return value;
}
__attribute__((noinline)) unsigned long long clear(unsigned i) {
    unsigned long long values[80];
    __builtin_memset(values, 0, sizeof values); values[i & 79u] = 17;
    return values[i & 79u] + values[(i & 79u) ^ 1u];
}
struct Self { int* ptr; int value; Self(int x): ptr(&value), value(x) {} };
__attribute__((noinline)) int publish(Self* s) { int* p = s->ptr; *p += 4; return s->value; }
int main() {
    volatile int n = 7;
    if (order != 12 || a != 3 || b != 5) return 1;
    Padded p; __builtin_memset(&p, 0x5a, sizeof p); p.tag = 'q'; p.value = n;
    Padded q = copy(p); if (q.tag != 'q' || q.value != 7) return 2;
    // Compare only semantic members: padding of a by-value copy is unspecified.
    unsigned char source[24], dest[24];
    for (unsigned i = 0; i < 24; ++i) source[i] = static_cast<unsigned char>(i + n);
    __builtin_memcpy(dest, source, sizeof source);
    __builtin_memmove(dest + 2, dest, 20);
    for (unsigned i = 2; i < 22; ++i) if (dest[i] != source[i-2]) return 3;
    Bits z; z.d = carrier(0x8000000000000000ULL);
    if (z.u != 0x8000000000000000ULL) return 4;
    Bits nan; nan.d = carrier(0x7ff8000000000012ULL);
    if (nan.u != 0x7ff8000000000012ULL) return 5;
    Aligned aligned = {{1, 2, 3, 4}};
    if ((reinterpret_cast<unsigned long>(&aligned) & 31u) != 0) return 6;
    Self self(n); if (publish(&self) != 11 || self.ptr != &self.value) return 7;
    if (clear(n) != 17 || aligned.values[3] != 4) return 8;
    return 0;
}
