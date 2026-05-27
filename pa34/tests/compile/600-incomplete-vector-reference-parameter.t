#include <vector>

struct S;

struct I
{
  virtual void f(const std::vector<S> &) = 0;
};

int main()
{
  return 0;
}
