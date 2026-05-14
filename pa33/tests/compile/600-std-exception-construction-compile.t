#include <exception>

int main()
{
  std::exception e;
  std::exception f(e);
  return f.what() ? 0 : 1;
}
