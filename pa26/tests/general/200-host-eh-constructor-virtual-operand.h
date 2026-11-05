int live = 0;
int calls = 0;
bool fail = false;
struct Part {
  int value;
  Part(int v = 0) : value(v) { ++live; }
  Part(const Part& p) : value(p.value) { ++live; }
  ~Part() noexcept { --live; }
  Part& operator=(const Part& p) { value = p.value; return *this; }
};
Part helper(Part a, Part b) { if (fail && ++calls == 2) throw 9; return Part(a.value + b.value); }
struct Base { virtual ~Base() {} virtual Part name() const = 0; };
struct Derived : Base {
  Part a, b;
  Derived() { a = helper(name(), Part(1)); b = helper(name(), Part(2)); }
  Part name() const override { return Part(7); }
};
int main() {
  { Derived d; if (d.a.value != 8 || d.b.value != 9) return 1; }
  if (live != 0) return 2;
  fail = true;
  try { Derived d; return 3; }
  catch (int value) { if (value != 9) return 4; }
  return live != 0;
}
