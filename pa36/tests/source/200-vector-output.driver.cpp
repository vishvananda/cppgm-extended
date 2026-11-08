#include <cstdint>
#include <cstdio>
#include <cstring>
using Vector = float __attribute__((vector_size(16)));
extern "C" Vector complement(Vector);
int main() {
    const uint32_t patterns[][4] = {
        {0x80000000,0x7fc12345,0x00000001,0xff800000},
        {0x11223344,0x55667788,0x99aabbcc,0xddeeff00}
    };
    for (const auto& pattern : patterns) {
        Vector input;
        std::memcpy(&input, pattern, 16);
        const Vector output = complement(input);
        uint32_t actual[4];
        std::memcpy(actual, &output, 16);
        for (unsigned i = 0; i < 4; ++i) if (actual[i] != ~pattern[i]) return 1;
    }
    std::puts("vector outputs: all 128 bits passed");
}
