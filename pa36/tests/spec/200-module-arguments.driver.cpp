#include <cstdio>
extern "C" long module_constant(long);
int main() {
    for (long value=-61; value<62; ++value) if(module_constant(value)!=value+23) return 1;
    std::puts("module typed constant survives LowIR transport");
}
