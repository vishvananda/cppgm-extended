struct Base
{
  virtual void destroy() = 0;

protected:
  Base()
  {
  }

  virtual ~Base()
  {
  }
};

struct Impl : Base
{
  static Base* create()
  {
    return new Impl();
  }

  Impl()
  {
  }

  void destroy()
  {
    delete this;
  }
};

int main()
{
  Base* base = Impl::create();
  base->destroy();
  return 0;
}
