struct Base
{
  virtual ~Base() {}
  virtual void destroy() {}
};

struct Derived : Base
{
  ~Derived() override {}

  void destroy() override
  {
    this->~Derived();
  }
};

int main()
{
  Derived d;
  d.destroy();
  return 0;
}
