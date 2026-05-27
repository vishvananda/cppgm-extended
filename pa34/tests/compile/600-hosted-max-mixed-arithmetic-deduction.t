#include <algorithm>

unsigned long grow(unsigned long current) {
  return std::max(current, 2 * current);
}

int main() {
  return grow(3) == 6 ? 0 : 1;
}
