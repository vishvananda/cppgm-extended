struct Base
{
  virtual Base * clone() { return this; }
};

struct Derived : Base
{
  virtual Derived * clone() { return this; }
};

int main()
{
  Derived d;
  Base * b = &d;
  return b->clone() == &d ? 0 : 1;
}
