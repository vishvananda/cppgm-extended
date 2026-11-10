#include <cstdio>
extern "C" long asm_max(long, long);
extern "C" unsigned asm_less(long, long);
extern "C" void asm_setmem(unsigned char*, long, long);
extern "C" unsigned asm_bts(unsigned long*, unsigned long);
extern "C" unsigned long asm_bswap(unsigned long);
extern "C" void asm_scans(unsigned long, unsigned long*);
extern "C" unsigned long asm_popcnt(unsigned long);
extern "C" unsigned long asm_tzcnt(unsigned long);
extern "C" unsigned long asm_lzcnt(unsigned long);
extern "C" unsigned long asm_shld(unsigned long, unsigned long, unsigned char);
extern "C" unsigned long asm_shrd(unsigned long, unsigned long, unsigned char);
extern "C" bool asm_cas128(unsigned __int128*, unsigned long*, unsigned long, unsigned long);
static void cpuid(unsigned leaf, unsigned subleaf, unsigned* result) {
    asm volatile("cpuid" : "=a"(result[0]), "=b"(result[1]), "=c"(result[2]), "=d"(result[3])
        : "a"(leaf), "c"(subleaf));
}
static unsigned popcount(unsigned long x) {
    unsigned count = 0;
    for (; x != 0; x >>= 1) count += x & 1;
    return count;
}
static unsigned trailing(unsigned long x) {
    if (x == 0) return 64;
    unsigned count = 0;
    while ((x & 1) == 0) { ++count; x >>= 1; }
    return count;
}
static unsigned leading(unsigned long x) {
    if (x == 0) return 64;
    unsigned count = 0;
    while ((x & (1ul << 63)) == 0) { ++count; x <<= 1; }
    return count;
}
int main() {
    for (long a = -31; a < 32; ++a) for (long b = -17; b < 18; ++b) {
        if (asm_max(a, b) != (a < b ? b : a)) return 1;
        if (asm_less(a, b) != unsigned(a < b)) return 2;
        unsigned char bytes[3] = {17, 99, 23};
        asm_setmem(bytes + 1, a, b);
        if (bytes[0] != 17 || bytes[1] != unsigned(a > b) || bytes[2] != 23) return 19;
    }
    unsigned info[4];
    cpuid(1, 0, info);
    bool pop = (info[2] & (1u << 23)) != 0;
    bool cx16 = (info[2] & (1u << 13)) != 0;
    cpuid(0, 0, info);
    bool bmi = false;
    if (info[0] >= 7) { cpuid(7, 0, info); bmi = (info[1] & 8) != 0; }
    cpuid(0x80000000u, 0, info);
    bool abm = false;
    if (info[0] >= 0x80000001u) { cpuid(0x80000001u, 0, info); abm = (info[2] & 32) != 0; }
    for (unsigned i = 0; i < 128; ++i) {
        unsigned long x = (0x123456789abcdef1ul * i) ^ (1ul << (i & 63));
        unsigned long swapped = 0;
        for (unsigned b = 0; b < 8; ++b) swapped = (swapped << 8) | ((x >> (b * 8)) & 255);
        if (asm_bswap(x) != swapped) return 3;
        if (x != 0) {
            unsigned long found[2];
            asm_scans(x, found);
            if (found[0] != trailing(x) || found[1] != 63 - leading(x)) return 4;
        }
        if (pop && asm_popcnt(x) != popcount(x)) return 5;
        if (bmi && asm_tzcnt(x) != trailing(x)) return 6;
        if (abm && asm_lzcnt(x) != leading(x)) return 7;
        for (unsigned count = 1; count < 64; ++count) {
            unsigned long other = x ^ 0xfedcba9876543210ul;
            if (asm_shld(x, other, count) != ((x << count) | (other >> (64-count)))) return 8;
            if (asm_shrd(x, other, count) != ((x >> count) | (other << (64-count)))) return 9;
            unsigned long changed = x;
            if (asm_bts(&changed, count) != ((x >> count) & 1)) return 10;
            if (changed != (x | (1ul << count))) return 11;
        }
    }
    if (pop && asm_popcnt(0) != 0) return 12;
    if (bmi && asm_tzcnt(0) != 64) return 13;
    if (abm && asm_lzcnt(0) != 64) return 14;
    if (cx16) {
        alignas(16) unsigned __int128 value = (static_cast<unsigned __int128>(11) << 64) | 7;
        unsigned long expected[2] = {7, 11};
        if (!asm_cas128(&value, expected, 13, 17)) return 15;
        if (value != ((static_cast<unsigned __int128>(17) << 64) | 13)) return 16;
        if (asm_cas128(&value, expected, 19, 23)) return 17;
        if (expected[0] != 13 || expected[1] != 17) return 18;
    }
    std::puts("conditional selection, bit operations, scans, double shifts and wide CAS passed");
}
