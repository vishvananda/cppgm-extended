#include <initializer_list>

template<class T>
struct Bag
{
  Bag(std::initializer_list<T> values);
  int size;
};

template<class T>
Bag<T>::Bag(std::initializer_list<T> values) :
  size(static_cast<int>(values.size()))
{}

Bag<int> make_bag()
{
  return {1, 2, 3};
}

int main()
{
  Bag<int> bag = make_bag();
  return bag.size == 3 ? 0 : 1;
}
