// VALIDATION: compile-pass
// N3485 focus: hosted integration sentinel

#include <functional>

int main()
{
  std::function<int(int)> fn;
  fn = [](int x) { return x + 1; };
  return fn(4) == 5 ? 0 : 1;
}
