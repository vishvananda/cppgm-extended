// P1825R0 (P1155 DR): retain a successful rvalue overload selection for
// by-value sinks, derived-to-base returns, and const-reference copy fallbacks.
// expect exit 0
struct Value {
  int tag;
  Value() : tag(1) {}
  Value(const Value&) : tag(2) {}
  Value(Value&&) : tag(3) {}
};
struct Sink {
  int tag;
  Sink(Value value) : tag(value.tag) {}
};
Sink sink() { Value value; return value; }
struct Base {
  int tag;
  Base() : tag(4) {}
  Base(const Base&) : tag(5) {}
  Base(Base&&) : tag(6) {}
};
struct Derived : Base {};
Base slice() { Derived value; return value; }
struct Copy {
  int tag;
  Copy() : tag(7) {}
  Copy(Copy&) : tag(8) {}
  Copy(const Copy&) : tag(9) {}
};
Copy copy_parameter(Copy value) { return value; }
int main() {
  if (sink().tag != 3 || slice().tag != 6) return 1;
  return copy_parameter(Copy()).tag == 9 ? 0 : 2;
}
