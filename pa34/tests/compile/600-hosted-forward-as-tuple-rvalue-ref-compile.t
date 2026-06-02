#include <tuple>
#include <type_traits>
static_assert(std::is_same<decltype(std::forward_as_tuple(7)), std::tuple<int&&> >::value, "forward_as_tuple(rvalue) -> tuple<int&&>");
struct S { int v; };
int main() {
  S s; s.v = 7;
  auto t = std::forward_as_tuple(static_cast<S &&>(s));
  S * p = &std::get<0>(t);
  return (p == &s && std::get<0>(t).v == 7) ? 0 : 1;
}
