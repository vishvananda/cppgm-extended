template<typename T>
struct Wrap
{
  typedef T type;
};

template<typename T>
int choose(typename Wrap<T>::type)
{
  return 1;
}

int main()
{
  return choose(1);
}
