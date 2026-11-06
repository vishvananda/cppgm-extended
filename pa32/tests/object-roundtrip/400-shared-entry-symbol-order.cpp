int observed;

struct Base
{
  int value;
  Base(int);
  ~Base();
};

Base::Base(int x) : value(x) {}
Base::~Base() { observed += value; }

int first = 11;

struct Derived : Base
{
  Derived(int x) : Base(x) {}
};

int second = 13;

int main()
{
  {
    Base base(second);
    Derived derived(first);
  }
  return observed == 24 ? 0 : 1;
}
