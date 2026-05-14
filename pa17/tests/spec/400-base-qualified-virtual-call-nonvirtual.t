// VALIDATION: run-pass
// N3485 focus: 10.3 [class.virtual]

struct Base
{
  virtual int f()
  {
    return 1;
  }

  virtual ~Base() {}
};

struct Derived : Base
{
  int f() override
  {
    return Base::f();
  }
};

int main()
{
  Derived d;
  return d.f() == 1 ? 0 : 1;
}
