#include <cstdio>
extern "C" long module_typed_a(long);
extern "C" long module_typed_b(long);
extern "C" long module_typed_caller(long);
int main() {
    for(long value=-31;value<32;++value) {
        if(module_typed_a(value)!=value+11)return 1;
        if(module_typed_b(value)!=value+23)return 2;
        if(module_typed_caller(value)!=value*2+34)return 3;
    }
    std::puts("typed module definitions and ordinary callers keep symbol identity");
}
