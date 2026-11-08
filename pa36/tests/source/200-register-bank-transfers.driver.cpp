#include <cstdint>
#include <cstdio>
#include <cstring>
extern "C" float flip32(float);
extern "C" double flip64(double);
extern "C" unsigned long mix64(unsigned long, unsigned long);
extern "C" unsigned mix32(unsigned, unsigned);
int main() {
    const uint32_t words[] = {0, 0x80000000, 0x3f800000, 0x7f800000, 0x7f812345, 0xffffffff};
    const uint64_t longs[] = {0, 0x8000000000000000ULL, 0x3ff0000000000000ULL,
        0x7ff0000000000000ULL, 0x7ff0000000000001ULL, 0xffffffffffffffffULL};
    for (uint32_t bits : words) {
        float value;
        std::memcpy(&value, &bits, sizeof value);
        float result = flip32(value);
        uint32_t observed;
        std::memcpy(&observed, &result, sizeof result);
        if (observed != (bits ^ 0x80000000u)) return 1;
        for (uint32_t other : words) if (mix32(bits, other) != (bits ^ other)) return 2;
    }
    for (uint64_t bits : longs) {
        double value;
        std::memcpy(&value, &bits, sizeof value);
        double result = flip64(value);
        uint64_t observed;
        std::memcpy(&observed, &result, sizeof result);
        if (observed != (bits ^ 0x8000000000000000ULL)) return 3;
        for (uint64_t other : longs) if (mix64(bits, other) != (bits ^ other)) return 4;
    }
    std::puts("GPR/XMM transfers preserve scalar bits, NaN payloads and signed zero passed");
}
