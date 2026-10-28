// N3485 focus: 5.19 [expr.const] p2; an unknown reference needs its value.
struct Tag { constexpr int get() const { return 42; } };
int classify(const Tag& mode) {
  constexpr int value = mode.get();
  return value;
}
