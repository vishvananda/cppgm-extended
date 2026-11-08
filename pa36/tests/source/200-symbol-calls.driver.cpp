#include <cstdio>
#include <cstdint>
extern "C" unsigned long assembly_call();
extern "C" void assembly_prepare() {}
extern "C" __attribute__((noinline)) unsigned long assembly_callback() {
    return 100 + (reinterpret_cast<uintptr_t>(__builtin_frame_address(0)) & 15);
}
int main() {
    for (unsigned i = 0; i < 16; ++i) if (assembly_call() != 100) return 1;
    std::puts("symbol call relocations preserve declared clobbers and stack alignment");
}
