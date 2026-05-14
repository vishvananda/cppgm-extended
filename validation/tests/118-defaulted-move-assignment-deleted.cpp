struct S
{
  S(int value = 0) : value(value) {}
  S & operator=(S &&) = default;

  const int value;
};

int main()
{
  S a(1);
  S b(2);
  a = static_cast<S &&>(b);
  return 0;
}
