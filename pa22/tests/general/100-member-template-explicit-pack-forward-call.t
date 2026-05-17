template<class T>
T&& forward(T& t)
{
  return static_cast<T&&>(t);
}

template<class KeyT, class... Args>
int count(Args&&...)
{
  return sizeof...(Args);
}

struct S {
  template<class... Args>
  int g(Args&&... args)
  {
    return count<int>(forward<Args>(args)...);
  }
};

int main()
{
  S s;
  return s.g(7) != 1;
}
