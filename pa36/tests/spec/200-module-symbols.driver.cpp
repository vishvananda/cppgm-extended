#include <cstdio>
extern "C" long module_callback(long value) { return value*7+3; }
extern "C" long (*module_function_pointer)(long);
int main() {
    if(module_function_pointer!=module_callback) return 1;
    for(long value=-61; value<62; ++value) if(module_function_pointer(value)!=value*7+3)return 2;
    std::puts("module typed symbols retain object identity");
}
