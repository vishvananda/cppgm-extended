template<class T>
struct StripReference
{
  typedef T type;
};

template<class T>
struct StripReference<T &>
{
  typedef T type;
};

template<class T>
using strip_reference_t = typename StripReference<T>::type;

template<class... T>
struct Box
{};

template<class... T>
Box<strip_reference_t<T>...> make_box(T &&...)
{
  return Box<strip_reference_t<T>...>();
}

int main()
{
  int first = 1;
  int second = 2;
  int third = 3;
  Box<int, int, int> value = make_box(first, second, third);
  (void)value;
  return 0;
}
