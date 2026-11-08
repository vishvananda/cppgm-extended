// P0170R1: constexpr closure construction and invocation use the stored
// captures, including references, initialized captures and captured this.
constexpr int copied(int value) {
  auto closure = [value](int extra) constexpr { return value + extra; };
  value = 99;
  return closure(3);
}
constexpr int referenced() {
  int value = 2;
  auto closure = [&alias = value]() { return ++alias; };
  closure();
  return closure();
}
constexpr int mutable_capture() {
  auto closure = [value = 2]() mutable constexpr noexcept { return ++value; };
  return closure() + closure();
}
constexpr auto returned(int value) { return [value]() { return value + 1; }; }
constexpr int nested() {
  int value = 2;
  auto closure = [value]() { return [value]() { return value + 1; }(); };
  return closure();
}
constexpr int array_capture() {
  int values[2] = {3, 4};
  auto closure = [values]() { return values[0] + values[1]; };
  values[0] = 99;
  return closure();
}
struct Copy {
  int value;
  constexpr Copy(int n): value(n) {}
  constexpr Copy(const Copy& source): value(source.value + 1) {}
};
constexpr int class_capture() {
  Copy value(2);
  auto closure = [value]() { return value.value; };
  return closure();
}
constexpr int class_array_capture(int first) {
  Copy values[2] = {{first}, {2}};
  auto closure = [values]() { return values[0].value + values[1].value; };
  return closure();
}
struct Owner {
  int value;
  constexpr int get() const { return [this]() { return value + 1; }(); }
};
constexpr auto initialized = [value = 7]() { return value; };
static_assert([]() constexpr { return 7; }() == 7, "explicit");
static_assert(copied(4) == 7 && referenced() == 4, "capture identity");
static_assert(mutable_capture() == 7 && returned(6)() == 7, "closure state");
static_assert(nested() == 3 && array_capture() == 7, "nested and array copies");
static_assert(class_capture() == 3 && Owner{6}.get() == 7, "class objects");
static_assert(class_array_capture(1) == 5, "element copy constructors");
static_assert(initialized() == 7, "namespace initialized capture");
int main() {
  volatile int input = 4;
  return copied(input) == 7 && referenced() == 4 && mutable_capture() == 7
    && returned(6)() == 7 && nested() == 3 && array_capture() == 7
    && class_capture() == 3 && class_array_capture(input - 3) == 5
    && Owner{6}.get() == 7 && initialized() == 7 ? 0 : 1;
}
