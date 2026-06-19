#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

static_assert(
  std::is_same<decltype(std::forward_as_tuple(1UL)),
               std::tuple<unsigned long&&> >::value,
  "forward_as_tuple(rvalue) keeps an rvalue reference after <string>");

std::tuple<unsigned long&&> make_rvalue_tuple()
{
  return std::forward_as_tuple(1UL);
}

int main()
{
  return 0;
}
