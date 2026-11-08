#include <cstdio>
extern "C" void mask_lowir_8(unsigned char, unsigned char*);
extern "C" void mask_lowir_16(unsigned short, unsigned short*);
extern "C" void mask_lowir_32(unsigned, unsigned*);
extern "C" void mask_lowir_64(unsigned long long, unsigned long long*);
template<class T> struct Guarded { T before, value, after; };
int main() {
    if (!__builtin_cpu_supports("avx512f") || !__builtin_cpu_supports("avx512bw") || !__builtin_cpu_supports("avx512dq")) {
        std::puts("AVX-512 unavailable; typed mask encoding inspection retained");
        return 0;
    }
    {
        Guarded<unsigned char> bytes = {17, 0, 29};
        const unsigned char value = static_cast<unsigned char>(0xb5c713e983d76a5fULL);
        mask_lowir_8(value, &bytes.value);
        if (bytes.before != 17 || bytes.value != value || bytes.after != 29) return 8;
    }
    {
        Guarded<unsigned short> bytes = {17, 0, 29};
        const unsigned short value = static_cast<unsigned short>(0xb5c713e983d76a5fULL);
        mask_lowir_16(value, &bytes.value);
        if (bytes.before != 17 || bytes.value != value || bytes.after != 29) return 16;
    }
    {
        Guarded<unsigned> bytes = {17, 0, 29};
        const unsigned value = static_cast<unsigned>(0xb5c713e983d76a5fULL);
        mask_lowir_32(value, &bytes.value);
        if (bytes.before != 17 || bytes.value != value || bytes.after != 29) return 32;
    }
    {
        Guarded<unsigned long long> bytes = {17, 0, 29};
        const unsigned long long value = static_cast<unsigned long long>(0xb5c713e983d76a5fULL);
        mask_lowir_64(value, &bytes.value);
        if (bytes.before != 17 || bytes.value != value || bytes.after != 29) return 64;
    }
    std::puts("Typed mask operands preserve every byte and exclude clobbered units");
}
