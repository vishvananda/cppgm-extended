#include <string>
#include <tuple>
#include <utility>

struct V {};

std::pair<std::string const, V> f(std::string const &s, V const &v) {
  std::tuple<std::string const &> first(s);
  std::tuple<V const &> second(v);
  return std::pair<std::string const, V>(std::piecewise_construct, first, second);
}

int main() { return 0; }
