struct Inner
{
  int a;
  int b;
};

struct Outer
{
  Inner inner;
  int c;
};

int main()
{
  Outer value = {1, 2, 3};
  return value.inner.a == 1 && value.inner.b == 2 && value.c == 3 ? 0 : 1;
}
