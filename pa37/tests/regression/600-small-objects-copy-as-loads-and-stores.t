// A copy or a zero fill of at most 64 bytes is loads and stores, with every
// load before the first store: no bulk memory operation, whose bounds the
// engine checks. A larger one stays a bulk operation.
struct Small { long long a; int b; short c; char d; };
struct Large { char bytes[96]; };
extern "C" __attribute__((noinline)) void copy_small(Small* to, const Small* from) {
    *to = *from;
}
extern "C" __attribute__((noinline)) void clear_small(Small* to) {
    *to = Small();
}
extern "C" __attribute__((noinline)) void copy_large(Large* to, const Large* from) {
    *to = *from;
}
extern "C" __attribute__((noinline)) void shift(Small* items) {
    items[1] = items[0];
}
int main() {
    Small a = {1234567890123LL, -7, 300, 'x'}, b = {0, 0, 0, 0};
    copy_small(&b, &a);
    if (b.a != a.a || b.b != -7 || b.c != 300 || b.d != 'x') return 1;
    copy_small(&b, &b);
    if (b.a != a.a || b.d != 'x') return 2;
    Small pair[2] = {a, b};
    pair[0].b = 11;
    shift(pair);
    if (pair[1].b != 11 || pair[1].a != a.a) return 3;
    clear_small(&b);
    if (b.a != 0 || b.b != 0 || b.c != 0 || b.d != 0) return 4;
    Large x, y;
    for (int i = 0; i < 96; ++i) x.bytes[i] = (char)(i * 7);
    copy_large(&y, &x);
    for (int i = 0; i < 96; ++i)
        if (y.bytes[i] != (char)(i * 7)) return 5;
    return 0;
}
