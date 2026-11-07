// expect exit 0
// N3485 focus: 3.2p2-p3, 9.8p1 and 5.1.2p11; constexpr scalar value versus object use.
// Regression family from GCC g++.dg/cpp0x/constexpr-__func__3.C (PR c++/105809).
int target(int n) { return n + 3; }
int main() {
  constexpr int (*pointer)(int) = target;
  struct Local { int read() { return pointer(7); } };
  return Local().read() != 10;
}
