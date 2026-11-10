#include <cstdio>
#include <cstdint>
extern "C" const unsigned* inline_data(unsigned*);
extern "C" unsigned inline_balign();
extern "C" unsigned inline_numeric_address();
extern "C" const unsigned char* inline_target_align();
int main() {
    unsigned observed = 0;
    const unsigned* address = inline_data(&observed);
    if (reinterpret_cast<std::uintptr_t>(address) % 256 || observed != 0x8357c1e9u || *address != observed) return 1;
    const unsigned char* bytes = reinterpret_cast<const unsigned char*>(address);
    if (bytes[4] != 17 || bytes[5] != 29 || bytes[6] != 0xc3 || bytes[7] != 0xb7) return 2;
    for (unsigned i = 8; i < 16; ++i) if (bytes[i] != 0) return 3;
    if (inline_balign() != 73 || inline_numeric_address() != 0x76543210u) return 4;
    const unsigned char* target = inline_target_align();
    if (reinterpret_cast<std::uintptr_t>(target) % 64 || *target != 0x5a) return 5;
    std::puts("Inline data, local RIP addresses and alignment preserve label scope");
}
