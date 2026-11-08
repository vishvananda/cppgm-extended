#include <cstdio>
extern "C" void mask_8(unsigned char, unsigned char*);
extern "C" unsigned char mask_tied_8(unsigned char);
extern "C" void mask_16(unsigned short, unsigned short*);
extern "C" unsigned short mask_tied_16(unsigned short);
extern "C" void mask_32(unsigned, unsigned*);
extern "C" unsigned mask_tied_32(unsigned);
extern "C" void mask_64(unsigned long long, unsigned long long*);
extern "C" unsigned long long mask_tied_64(unsigned long long);
template<class T> struct Guarded { T before, value, after; };
int main() {
    if (!__builtin_cpu_supports("avx512f") || !__builtin_cpu_supports("avx512bw") || !__builtin_cpu_supports("avx512dq")) {
        std::puts("AVX-512 unavailable; mask encoding inspection retained");
        return 0;
    }
    {
        const unsigned char values[] = {0, static_cast<unsigned char>(-1), static_cast<unsigned char>(0x8357e9adb3c1d56fULL)};
        for (unsigned char value : values) {
            Guarded<unsigned char> guarded = {17, 0, 29};
            mask_8(value, &guarded.value);
            if (guarded.before != 17 || guarded.after != 29 || guarded.value != value || mask_tied_8(value) != value) return 8;
        }
    }
    {
        const unsigned short values[] = {0, static_cast<unsigned short>(-1), static_cast<unsigned short>(0x8357e9adb3c1d56fULL)};
        for (unsigned short value : values) {
            Guarded<unsigned short> guarded = {17, 0, 29};
            mask_16(value, &guarded.value);
            if (guarded.before != 17 || guarded.after != 29 || guarded.value != value || mask_tied_16(value) != value) return 16;
        }
    }
    {
        const unsigned values[] = {0, static_cast<unsigned>(-1), static_cast<unsigned>(0x8357e9adb3c1d56fULL)};
        for (unsigned value : values) {
            Guarded<unsigned> guarded = {17, 0, 29};
            mask_32(value, &guarded.value);
            if (guarded.before != 17 || guarded.after != 29 || guarded.value != value || mask_tied_32(value) != value) return 32;
        }
    }
    {
        const unsigned long long values[] = {0, static_cast<unsigned long long>(-1), static_cast<unsigned long long>(0x8357e9adb3c1d56fULL)};
        for (unsigned long long value : values) {
            Guarded<unsigned long long> guarded = {17, 0, 29};
            mask_64(value, &guarded.value);
            if (guarded.before != 17 || guarded.after != 29 || guarded.value != value || mask_tied_64(value) != value) return 64;
        }
    }
    std::puts("Mask values preserve widths, ties, early outputs and guard bytes");
}
