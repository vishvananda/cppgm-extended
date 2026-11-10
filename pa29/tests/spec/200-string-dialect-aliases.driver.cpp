#include <cstdio>
extern "C" void direct_copy_dwords(unsigned*, const unsigned*, unsigned long);
int main() {
    unsigned source[64], target[66];
    for (unsigned i=0; i<64; ++i) source[i] = 0x10203040u * (i+1);
    for (unsigned count=0; count<64; ++count) {
        for (unsigned& value : target) value = 0xa5a5a5a5u;
        direct_copy_dwords(target+1, source, count);
        if (target[0] != 0xa5a5a5a5u || target[count+1] != 0xa5a5a5a5u) return 1;
        for (unsigned i=0; i<count; ++i) if (target[i+1] != source[i]) return 2;
    }
    std::puts("Intel movsd string alias survives LowIR transport");
}
