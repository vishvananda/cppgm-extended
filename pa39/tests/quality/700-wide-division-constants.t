// Divisions and remainders by literals that no hardware divide computes. A
// 64-bit one takes the high half of a product by a magic number (SMULH or
// UMULH): signed with the dividend's correction and the sign bit added,
// unsigned with the 65-bit fix-up (7) or with the even divisor's zeros
// shifted out of the dividend first (14). A signed 32-bit one whose
// multiplier needs all 32 bits (120, 7) adds the dividend times 2^32 to the
// widening product of the multiplier as a negative word.
extern "C" __attribute__((noinline)) unsigned long quotients(const long* x, int n) {
    unsigned long s = 0;
    for (int i = 0; i < n; ++i)
        s += (unsigned long)(x[i] / 7);
    return s;
}

extern "C" __attribute__((noinline)) unsigned long negative_quotients(const long* x, int n) {
    unsigned long s = 0;
    for (int i = 0; i < n; ++i)
        s += (unsigned long)(x[i] / -1000);
    return s;
}

extern "C" __attribute__((noinline)) unsigned long remainders(const long* x, int n) {
    unsigned long s = 0;
    for (int i = 0; i < n; ++i)
        s += (unsigned long)(x[i] % 1000000007);
    return s;
}

extern "C" __attribute__((noinline)) unsigned long uquotients(const unsigned long* x, int n) {
    unsigned long s = 0;
    for (int i = 0; i < n; ++i)
        s += x[i] / 7;
    return s;
}

extern "C" __attribute__((noinline)) unsigned long ueven(const unsigned long* x, int n) {
    unsigned long s = 0;
    for (int i = 0; i < n; ++i)
        s += x[i] / 14;
    return s;
}

extern "C" __attribute__((noinline)) unsigned long uremainders(const unsigned long* x, int n) {
    unsigned long s = 0;
    for (int i = 0; i < n; ++i)
        s += x[i] % 10;
    return s;
}

extern "C" __attribute__((noinline)) unsigned words(const int* x, int n) {
    unsigned s = 0;
    for (int i = 0; i < n; ++i)
        s += (unsigned)(x[i] / 120);
    return s;
}

extern "C" __attribute__((noinline)) unsigned word_remainders(const int* x, int n) {
    unsigned s = 0;
    for (int i = 0; i < n; ++i)
        s += (unsigned)(x[i] % 7);
    return s;
}
