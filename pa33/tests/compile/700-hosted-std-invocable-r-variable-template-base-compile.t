#include <type_traits>
#include <string>

struct MakeString {
  std::string operator()();
};

int main() {
#if defined(_LIBCPP_VERSION) && _LIBCPP_VERSION >= 210000
  static_assert(std::__is_invocable_r_v<std::string, MakeString&>, "callable");
#endif
  return 0;
}
