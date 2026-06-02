#include <cmath>
#include <type_traits>
static_assert(std::is_same<decltype(std::ceil(1.0)), double>::value, "std::ceil(double) -> double");
int main() { return std::ceil(1.0f) == 1.0f ? 0 : 1; }
