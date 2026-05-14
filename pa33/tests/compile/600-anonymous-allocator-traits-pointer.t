#include <memory>

namespace {
struct LowIRBlock {};
std::allocator_traits<std::allocator<LowIRBlock>>::pointer p;
}

int main() { return 0; }
