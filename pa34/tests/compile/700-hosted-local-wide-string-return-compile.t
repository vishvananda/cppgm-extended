#include <string>
#include <type_traits>
static_assert(std::is_same<std::u16string::value_type, char16_t>::value, "u16string value_type");
static_assert(std::is_same<std::u32string::value_type, char32_t>::value, "u32string value_type");
std::u16string make_u16() { std::u16string value; return value; }
std::u32string make_u32() { std::u32string value; return value; }
int main() { return static_cast<int>(make_u16().size() + make_u32().size()); }
