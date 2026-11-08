#include <cstdio>
extern "C" unsigned long integer_mix(unsigned long, unsigned);
extern "C" unsigned long integer_unary(unsigned long, unsigned*);
extern "C" unsigned short integer_bytes(unsigned short);
int main() {
    const unsigned long words[] = {0, 1, 7, 0x8000000000000000UL, 0xffffffffffffffffUL, 0x123456789abcdef0UL};
    for (unsigned long value : words) {
        for (unsigned count = 0; count < 130; ++count) {
            unsigned long product = value * static_cast<unsigned long>(-7L);
            unsigned long rotated = (product << 13) | (product >> 51);
            if (integer_mix(value, count) != (rotated >> (count & 63))) return 1;
        }
        unsigned nonzero = 7;
        unsigned long expected = value + 1;
        if (integer_unary(value, &nonzero) != expected || nonzero != (expected != 0)) return 2;
    }
    for (unsigned value = 0; value < 65536; value += 13)
        if (integer_bytes(value) != static_cast<unsigned short>((value << 8) | (value >> 8))) return 3;
    std::puts("multiply, shifts, unary operations, flags and high-byte aliases passed");
}
