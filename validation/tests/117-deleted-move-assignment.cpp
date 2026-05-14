struct S
{
  S() {}
  S & operator=(const S &) = default;
  S & operator=(S &&) = delete;
};

int main()
{
  S a;
  S b;
  a = static_cast<S &&>(b);
  return 0;
}
