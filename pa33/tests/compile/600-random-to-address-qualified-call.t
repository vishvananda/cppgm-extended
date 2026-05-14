#include <random>

int main() {
  int x = 0;
  return *std::__to_address(&x);
}
