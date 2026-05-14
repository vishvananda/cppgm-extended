template<typename T>
struct A
{
  typedef int type;

  void f()
  {
    typename A<T>::other value = 0;
    (void)value;
  }
};

int main()
{
  A<int> a;
  a.f();
  return 0;
}
