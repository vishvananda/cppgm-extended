#include <cstdio>
extern "C" long module_dialect(long);
extern "C" long module_dialect_second(long);
int main() {
    for (long value=-63;value<64;++value) if (module_dialect(value)!=value+8 || module_dialect_second(value)!=3*value+8) return 1;
    std::puts("module dialect switches retain label scope across syntax directives");
}
