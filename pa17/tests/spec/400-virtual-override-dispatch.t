// VALIDATION: run-pass
// N3485 focus: 10.3 [class.virtual]

struct Base
{
  virtual int f() const
  {
    return 1;
  }

  virtual ~Base() {}
};

struct Derived : Base
{
  int f() const override
  {
    return 2;
  }
};

int main()
{
  Derived d;
  Base & b = d;
  return b.f() == 2 ? 0 : 1;
}
