template<class Ptr>
struct pointer_traits
{
  template<class U>
  struct rebind_pointer
  {
    typedef U * type;
  };
};

template<class Pointer>
struct iterator
{
  typedef typename pointer_traits<Pointer>::template rebind_pointer<int>::type pointer;
};

int expect(int *);

int main()
{
  iterator<char>::pointer p = 0;
  return expect(p);
}
