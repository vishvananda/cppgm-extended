#include <cstdio>
extern "C" long module_add(long);
extern "C" long module_read(long);
extern "C" long module_call(long);
extern "C" long module_data[3];
extern "C" long* module_pointer;
extern "C" long module_callback(long value) { return value * 5 + 1; }
int main() {
    if (module_data[0] != 11 || module_data[1] != 13 || module_data[2] != 17) return 1;
    if (module_pointer != module_data+1) return 2;
    if ((reinterpret_cast<unsigned long>(module_data) & 15) != 0) return 3;
    for (long x=-61; x<62; ++x) {
        if (module_add(x) != x+7) return 4;
        if (module_read(x) != x+20) return 5;
        if (module_call(x) != x*5+1) return 6;
    }
    std::puts("module definitions, C++ callers, typed object relocations and callbacks passed");
}
