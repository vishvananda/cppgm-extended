#include <cstdio>
extern "C" long asm_s8(signed char);
extern "C" unsigned long asm_u8(unsigned char);
extern "C" long asm_s16(short);
extern "C" unsigned long asm_u16(unsigned short);
extern "C" long asm_s32(int);
extern "C" unsigned asm_high(unsigned short);
extern "C" short asm_s8_16(signed char);
extern "C" long asm_memory_s8(const signed char*);
int main() {
    for (int value = -128; value < 128; ++value) {
        signed char byte = static_cast<signed char>(value);
        if (asm_s8(byte) != value) return 1;
        if (asm_u8(static_cast<unsigned char>(value)) != static_cast<unsigned char>(value)) return 2;
        if (asm_s8_16(byte) != value) return 3;
        if (asm_memory_s8(&byte) != value) return 4;
    }
    for (unsigned value = 0; value < 65536; value += 71) {
        if (asm_s16(static_cast<short>(value)) != static_cast<short>(value)) return 5;
        if (asm_u16(static_cast<unsigned short>(value)) != value) return 6;
        if (asm_high(static_cast<unsigned short>(value)) != (value >> 8)) return 7;
    }
    int values[] = {0, 1, -1, -2147483647 - 1, 2147483647, -123456789};
    for (int value : values) if (asm_s32(value) != static_cast<long>(value)) return 8;
    std::puts("mixed-width sign/zero extension and high-byte sources passed");
}
