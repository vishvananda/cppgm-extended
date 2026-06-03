#include <random>
#include <type_traits>
static_assert(std::mt19937::default_seed == 5489u, "<random> mt19937 default_seed");
int main() { int x = 0; return *std::__to_address(&x); }
