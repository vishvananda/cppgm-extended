#include <functional>

int main()
{
  std::function<void(unsigned long)> f = [](unsigned long) {};
  f(0);
}
