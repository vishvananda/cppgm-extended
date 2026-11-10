#include <cstdio>
extern "C" const unsigned char macro_control[];
int main() {
    const unsigned char expected[] = {7,1,2,4,5,6,11,17,23,19,31,37,41,43,47,53,53,53,53};
    for (unsigned i=0;i<sizeof expected;++i) if (macro_control[i]!=expected[i]) {
        std::printf("byte %u: expected %u, got %u\n",i,expected[i],macro_control[i]);
        return 1;
    }
    std::puts("nested macros, block exit, defaults, skipped branches and string conditions passed");
}
