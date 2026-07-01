// VALIDATION: compile-pass
// N3485 focus: 7.3.3 [namespace.udecl], 10.2 [class.member.lookup]

struct Base
{
  int f(int) { return 1; }
};

struct Derived : Base
{
  using Base::f;

  int f(double) { return 2; }
};

int main()
{
  Derived d;
  return d.f(1) == 1 && d.f(1.5) == 2 ? 0 : 1;
}
