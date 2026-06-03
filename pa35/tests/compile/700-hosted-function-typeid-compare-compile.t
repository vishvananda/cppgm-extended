#include <functional>
#include <type_traits>
static_assert(std::is_same<std::function<int()>::result_type, int>::value, "std::function<int()>::result_type");
int one() { return 1; }
int main() { std::function<int()> g = one; return 0; }
