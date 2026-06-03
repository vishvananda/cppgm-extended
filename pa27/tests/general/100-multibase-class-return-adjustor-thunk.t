struct S { int value; };
struct B1 { virtual int get() = 0; virtual ~B1() {} };
struct B2 { virtual S current() const = 0; virtual ~B2() {} };
struct D : B1, B2 {
  int get() { return 0; }
  S current() const { S out = {7}; return out; }
};
int main() { D d; B2 & b = d; return b.current().value - 7; }
