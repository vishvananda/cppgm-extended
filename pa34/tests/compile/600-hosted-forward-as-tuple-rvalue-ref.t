#include <tuple>
#include <string>

int main()
{
  auto t = std::forward_as_tuple(std::string("x"));
  (void)t;
  return 0;
}
