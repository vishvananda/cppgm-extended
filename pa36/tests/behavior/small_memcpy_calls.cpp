// Literal memcpy calls, including unaligned and odd sizes, must copy exactly
// the requested bytes and return the destination at every optimization level.
// Dynamic copies, indirect calls and overlapping memmove remain controls.
#include <cstdio>
#include <cstring>

template<unsigned N>
__attribute__((noinline)) void* copy_fixed(void* to, const void* from) {
    return std::memcpy(to, from, N);
}

__attribute__((noinline)) void copy_unused(void* to, const void* from) {
    std::memcpy(to, from, 13);
}

__attribute__((noinline)) void* copy_dynamic(void* to, const void* from, unsigned n) {
    return std::memcpy(to, from, n);
}

__attribute__((noinline)) void* move_overlapping(void* to, const void* from) {
    return std::memmove(to, from, 13);
}

template<unsigned N>
unsigned check() {
    unsigned char from[48], to[48];
    for (unsigned i = 0; i < 48; ++i) { from[i] = static_cast<unsigned char>(i * 7 + 3); to[i] = 0xa5; }
    unsigned bad = copy_fixed<N>(to + 3, from + 1) != to + 3;
    for (unsigned i = 0; i < 48; ++i)
        bad += to[i] != (i >= 3 && i < N + 3 ? from[i - 2] : 0xa5);
    return bad;
}

int main() {
    unsigned bad = check<0>() + check<1>() + check<2>() + check<3>() + check<5>() + check<8>() +
        check<13>() + check<16>() + check<17>() + check<31>() + check<32>() + check<33>();
    unsigned char from[32], to[32];
    for (unsigned i = 0; i < 32; ++i) { from[i] = static_cast<unsigned char>(i + 1); to[i] = 0; }
    copy_unused(to + 1, from + 2);
    bad += copy_dynamic(to + 14, from + 3, 5) != to + 14;
    for (unsigned i = 0; i < 13; ++i) bad += to[i + 1] != from[i + 2];
    for (unsigned i = 0; i < 5; ++i) bad += to[i + 14] != from[i + 3];
    bad += move_overlapping(from + 2, from) != from + 2;
    for (unsigned i = 0; i < 13; ++i) bad += from[i + 2] != i + 1;
    void* (*indirect)(void*, const void*, unsigned) = copy_dynamic;
    bad += indirect(to, from, 4) != to;
    for (unsigned i = 0; i < 4; ++i) bad += to[i] != from[i];
    std::printf("%u\n", bad);
    return bad != 0;
}
