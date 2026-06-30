template<class T>
struct iterator_traits
{
};

template<class T>
struct iterator_traits<const T *>
{
  typedef long difference_type;
};

template<class I>
I next(I value, typename iterator_traits<I>::difference_type count)
{
  return value + count;
}

int main()
{
  const char text[4] = {'a', 'b', 'c', 0};
  return *next(text, 2) - 'c';
}
