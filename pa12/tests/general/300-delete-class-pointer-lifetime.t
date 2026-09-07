int c;

struct A {
  A()
  {
    ++c;
  }

  ~A()
  {
    --c;
  }
};

int main()
{
  A * a = new A();
  delete a;
  return c;
}
