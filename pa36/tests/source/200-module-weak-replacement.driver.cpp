#include <cstdio>
extern "C" long module_weak(long value) { return value+31; }
extern "C" long module_wrap(long);
int main() {
    for(long value=-31;value<32;++value) if(module_wrap(value)!=value+31)return 1;
    std::puts("module calls preserve weak symbol replacement");
}
