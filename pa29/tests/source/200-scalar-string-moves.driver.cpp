#include <cstdio>
#include <cstring>
extern "C" float asm_float(float);
extern "C" double asm_double(double);
extern "C" void asm_store_float(float*, float);
extern "C" void asm_store_double(double*, double);
extern "C" void asm_copy_words(unsigned*, const unsigned*, unsigned long);
extern "C" void asm_set_words(unsigned*, unsigned, unsigned long);
extern "C" unsigned long asm_zero_move(unsigned*);
extern "C" unsigned long asm_absolute_move(unsigned*);
int main() {
    unsigned float_bits[] = {0, 0x80000000u, 1, 0x3f800000u, 0x7f800000u, 0xff800000u, 0x7fc12345u, 0x7f812345u};
    for (unsigned bits : float_bits) {
        float value;
        std::memcpy(&value, &bits, 4);
        float result = asm_float(value);
        unsigned observed;
        std::memcpy(&observed, &result, 4);
        if (observed != bits) return 1;
        float storage[3];
        std::memset(storage, 0xa5, sizeof storage);
        asm_store_float(storage+1, value);
        unsigned words[3];
        std::memcpy(words, storage, sizeof words);
        if (words[0] != 0xa5a5a5a5u || words[1] != bits || words[2] != 0xa5a5a5a5u) return 2;
    }
    unsigned long double_bits[] = {0, 1, 0x8000000000000000ul, 0x3ff0000000000000ul, 0x7ff0000000000000ul, 0x7ff8123456789abcul, 0x7ff0123456789abcul};
    for (unsigned long bits : double_bits) {
        double value;
        std::memcpy(&value, &bits, 8);
        double result = asm_double(value);
        unsigned long observed;
        std::memcpy(&observed, &result, 8);
        if (observed != bits) return 3;
        double storage[3];
        std::memset(storage, 0xa5, sizeof storage);
        asm_store_double(storage+1, value);
        unsigned long words[3];
        std::memcpy(words, storage, sizeof words);
        if (words[0] != 0xa5a5a5a5a5a5a5a5ul || words[1] != bits || words[2] != 0xa5a5a5a5a5a5a5a5ul) return 4;
    }
    for (unsigned count = 0; count < 64; ++count) {
        unsigned source[64], destination[66];
        for (unsigned i=0; i<64; ++i) source[i] = 0x12345678u * (i+1);
        for (unsigned& value : destination) value = 0xa5a5a5a5u;
        asm_copy_words(destination+1, source, count);
        if (destination[0] != 0xa5a5a5a5u || destination[count+1] != 0xa5a5a5a5u) return 5;
        for (unsigned i=0; i<count; ++i) if (destination[i+1] != source[i]) return 6;
        asm_set_words(destination+1, 0x87654321u, count);
        for (unsigned i=0; i<count; ++i) if (destination[i+1] != 0x87654321u) return 7;
        if (destination[0] != 0xa5a5a5a5u || destination[count+1] != 0xa5a5a5a5u) return 8;
    }
    unsigned carry = 0;
    if (asm_zero_move(&carry) != 0 || carry != 1) return 9;
    if (asm_absolute_move(&carry) != 1 || carry != 1) return 10;
    std::puts("scalar bit preservation, word strings and move flags passed");
}
