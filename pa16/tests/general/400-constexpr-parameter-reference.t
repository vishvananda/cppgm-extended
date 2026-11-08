// P2280R4 adopted DR: passing an unknown reference reads no object value.
struct Tag { constexpr int get() const { return 42; } };
int classify(const Tag& mode) {
  constexpr int value = mode.get();
  return value;
}
