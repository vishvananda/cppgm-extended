// An aggregate passes and returns through memory on this target. The callee
// reads a by-value parameter in the caller's copy; a caller passes the slot
// that it filled for one argument; and a result is built in the caller's
// result object, which a call fills in the slot that takes the result.
struct Pair { long long first; long long second; };
struct Wide { long long a, b, c; };
extern "C" __attribute__((noinline)) long long total(Pair p) {
    return p.first + p.second;
}
extern "C" __attribute__((noinline)) Pair swapped(Pair p) {
    Pair result = {p.second, p.first};
    return result;
}
extern "C" __attribute__((noinline)) Wide spread(Pair p, long long c) {
    Wide w = {p.first, p.second, c};
    return w;
}
extern "C" __attribute__((noinline)) long long forward(const Pair* p) {
    return total(*p) + total(swapped(*p));
}
extern "C" __attribute__((noinline)) Wide chain(const Pair* p) {
    return spread(swapped(*p), 9);
}
extern "C" __attribute__((noinline)) long long mutate(Pair p) {
    p.first += 5;
    return total(p);
}
int main() {
    Pair p = {3, 40};
    if (total(p) != 43) return 1;
    Pair s = swapped(p);
    if (s.first != 40 || s.second != 3 || p.first != 3 || p.second != 40) return 2;
    if (forward(&p) != 86) return 3;
    Wide w = chain(&p);
    if (w.a != 40 || w.b != 3 || w.c != 9) return 4;
    if (mutate(p) != 48 || p.first != 3) return 5;
    for (int i = 0; i < 3; ++i) {
        p = swapped(p);
        if (total(p) != 43) return 6;
    }
    return p.first != 40;
}
