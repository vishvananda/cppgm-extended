// Constant arithmetic feeding variable operands; preserve narrow signedness,
// shifts and conversions, and do not fold a call that returns a constant.
#include <cstdio>
#include <cstdint>

static unsigned calls;
__attribute__((noinline)) unsigned width() { ++calls; return 4; }
__attribute__((noinline)) uint64_t masked(uint64_t value) {
    return value & ~static_cast<uint64_t>((1 << 4) - 1);
}
__attribute__((noinline)) uint64_t dynamic_mask(uint64_t value) {
    return value & ~static_cast<uint64_t>((1 << width()) - 1);
}
__attribute__((noinline)) uint64_t constants(uint64_t value) {
    return ((value ^ static_cast<uint32_t>(-1)) + (17 * 31 - 3)) &
        static_cast<uint64_t>(static_cast<unsigned char>(-1));
}
__attribute__((noinline)) int64_t signed_values(int64_t value) {
    return value + static_cast<signed char>(0x80) +
        static_cast<int64_t>(static_cast<unsigned char>(0xfe));
}
int main() {
    uint64_t hash = 0;
    for (unsigned i = 0; i < 1000; ++i) {
        hash = hash * 31 + masked(i * 1234567ull);
        hash = hash * 31 + dynamic_mask(i * 7654321ull);
        hash = hash * 31 + constants(i * 17171ull);
        hash = hash * 31 + static_cast<uint64_t>(signed_values(i));
    }
    std::printf("%llu %u\n", static_cast<unsigned long long>(hash), calls);
    return calls != 1000;
}
