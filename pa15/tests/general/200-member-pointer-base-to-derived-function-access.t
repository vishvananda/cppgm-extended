struct Base
{
  int f()
  {
    return 5;
  }
};

struct Derived : Base
{
};

int main()
{
  int (Base::* base_ptr)() = &Base::f;
  int (Derived::* derived_ptr)() = base_ptr;
  Derived d;
  return (d.*derived_ptr)() == 5 ? 0 : 1;
}
