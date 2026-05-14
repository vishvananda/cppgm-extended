#include <string>

struct Derived;

struct Base {
  std::size_t hash() const;
};

struct Derived : Base {
  std::size_t value;
};

std::size_t Base::hash() const
{
  return static_cast<Derived const&>(*this).value;
}

int main()
{
  Derived d{};
  d.value = 7;
  return d.hash() == 7 ? 0 : 1;
}
