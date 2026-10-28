// N3485 focus: 5.19 [expr.const] p3; a parameter cannot be a reference constant.
struct Tag {};
int classify(Tag mode) {
  constexpr const Tag& value = mode;
  return &value == &mode;
}
