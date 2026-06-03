#include <chrono>
#include <string_view>
#include <type_traits>
static_assert(std::chrono::seconds::period::num == 1 && std::chrono::seconds::period::den == 1, "chrono seconds is 1:1");
static_assert(std::is_same<std::string_view::value_type, char>::value, "string_view value_type");
