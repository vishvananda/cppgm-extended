#include <string>
#include <utility>

enum E
{
  A
};

template<class T>
std::pair<const std::string, E> make_pairish(T&& t)
{
  std::string local(std::forward<T>(t));
  return std::pair<const std::string, E>(std::forward<T>(t), A);
}

int main()
{
  std::pair<const std::string, E> p = make_pairish("(");
  return p.first.size() == 1 ? 0 : 1;
}
