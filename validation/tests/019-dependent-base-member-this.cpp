// VALIDATION: compile-pass
// N3485 focus: 14.6.2 [temp.dep], 14.6.4 [temp.dep.res]

template<typename T>
struct Base
{
  int value() const
  {
    return 7;
  }
};

template<typename T>
struct Derived : Base<T>
{
  int run() const
  {
    return this->value();
  }
};

int main()
{
  Derived<int> d;
  return d.run() == 7 ? 0 : 1;
}
