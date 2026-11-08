#include <cstdint>
#include <cstdio>
struct Block { unsigned low, high; };
extern "C" unsigned long read_block(const Block*);
extern "C" unsigned memory_update(unsigned, unsigned*);
int main() {
    const Block blocks[] = {{0x12345678, 0x87654321}, {0xffffffff, 0x80000000}};
    for (const Block& block : blocks) {
        if (read_block(&block) != (uint64_t(block.high) << 32 | block.low)) return 1;
    }
    const unsigned inputs[] = {0, 1, 1973, 0x80000000u, 0xffffffffu};
    for (unsigned input : inputs) {
        unsigned calls = 0;
        if (memory_update(input, &calls) != input + 24 || calls != 1) return 2;
    }
    std::puts("memory objects, volatile access and one address evaluation passed");
}
