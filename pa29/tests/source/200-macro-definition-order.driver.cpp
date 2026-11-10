#include <cstdio>
extern "C" long macro_source_early(long);
extern "C" long macro_source_late(long);
extern "C" long macro_source_instantiated(long);
extern "C" long macro_source_inlined(long);
int main() {
    for (long value=-63;value<64;++value) {
        if (macro_source_early(value)!=value+2 || macro_source_late(value)!=value+7) return 1;
        if (macro_source_instantiated(value)!=2*value+4 || macro_source_inlined(value)!=2*value+9) return 2;
    }
    std::puts("source macro environments survive deferred bodies, instantiation and inlining");
}
