// N3485 8.5.4p3, 8.5.3p5: a related list element can bind a mutable lvalue
// or rvalue reference without making an abstract temporary. expect exit 0
struct Abstract { virtual int value() const = 0; };
struct Concrete : Abstract { int value() const { return 7; } };
Concrete global;
int mutable_read(Abstract& value) { return &value == &global ? value.value() : 0; }
int rvalue_read(Abstract&& value) { return value.value(); }
int main() {
  if (mutable_read({global}) != 7) return 1;
  return rvalue_read({Concrete()}) == 7 ? 0 : 2;
}
