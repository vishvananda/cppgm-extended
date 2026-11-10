#include <cstdio>
extern "C" unsigned flags_alias(unsigned);
int main() {
    if (flags_alias(4) != 5 || flags_alias(~0u) != 1) return 1;
    std::puts("trap forms encode; flags clobber preserves outputs");
}
