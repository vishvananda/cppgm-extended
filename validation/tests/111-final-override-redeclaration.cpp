struct Base
{
  virtual int f() { return 1; }
};

struct Mid : Base
{
  int f() final { return 2; }
};

struct Derived : Mid
{
  int f() { return 3; }
};

int main()
{
  Derived d;
  return d.f();
}
