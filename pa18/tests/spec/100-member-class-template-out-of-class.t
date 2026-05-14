// VALIDATION: run-pass
// N3485 focus: 14.5.2 [temp.mem]

template<typename T>
struct outer
{
  template<typename U>
  struct inner;
};

template<typename T>
template<typename U>
struct outer<T>::inner
{
  static const int value = sizeof(T) + sizeof(U);
};

int main()
{
  return outer<int>::inner<char>::value ==
                 static_cast<int>(sizeof(int) + sizeof(char))
             ? 0
             : 1;
}
