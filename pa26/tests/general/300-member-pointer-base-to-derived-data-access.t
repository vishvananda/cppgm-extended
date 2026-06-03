struct Base
{
  int member;
};

struct Derived : Base
{
};

int main()
{
  int Base::* base_ptr = &Base::member;
  int Derived::* derived_ptr = base_ptr;
  Derived d;
  d.member = 5;
  return d.*derived_ptr == 5 ? 0 : 1;
}
