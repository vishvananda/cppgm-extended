#include <functional>

int one() { return 1; }

int main() {
  std::function<int()> g = one;
  return 0;
}
