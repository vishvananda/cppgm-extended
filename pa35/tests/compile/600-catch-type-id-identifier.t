#include <stdexcept>
#include <type_traits>
static_assert(std::is_base_of<std::exception, std::out_of_range>::value, "out_of_range : exception");
int main() { try { throw std::out_of_range("x"); } catch(std::out_of_range) { return 0; } }
