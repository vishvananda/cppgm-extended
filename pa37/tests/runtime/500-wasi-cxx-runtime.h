#include <cstdio>
static int destroyed;
struct Guard {
 ~Guard() noexcept { ++destroyed; }
};
int main() {
 for (int i = 0; i < 3; ++i) {
  try { Guard guard; throw i + 17; }
  catch (int value) { if (value != i + 17) return 1; }
 }
 std::printf("cleanup %d\n", destroyed);
 return destroyed == 3 ? 0 : 2;
}
