#include <iterator>

int main() {
  const char *p = "abc";
  const char *q = std::prev(p + 1);
  return q == p ? 0 : 1;
}
