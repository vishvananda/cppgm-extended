// N3485 focus: 5.19 [expr.const] p2; unknown subobject values cannot be read.
struct Tag { int value; constexpr int get() const { return value; } };
int classify(Tag mode) {
  constexpr int value = mode.get();
  return value;
}
