#include <cstdio>
#include <cstdint>
extern "C" void direct_symbol_call();
static unsigned calls;
extern "C" __attribute__((noinline)) void assembly_stack_callback() {
    if ((reinterpret_cast<uintptr_t>(__builtin_frame_address(0)) & 15) != 0) return;
    ++calls;
}
int main() {
    for (unsigned i = 0; i < 17; ++i) direct_symbol_call();
    if (calls != 17) return 1;
    std::puts("assembly stack effects align calls even in a frame-free body");
}
