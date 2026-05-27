struct Box
{
  void *p;

  Box() : p(0) {}
  Box(const Box & other) : p(other.p) {}
  ~Box() {}

  Box & operator=(const Box & other)
  {
    p = other.p;
    return *this;
  }
};

struct Primary
{
  virtual Box make(int tag) = 0;
  virtual ~Primary() {}
};

struct Secondary
{
  virtual bool check(int tag) = 0;
  virtual ~Secondary() {}
};

struct Derived : Primary, Secondary
{
  int value;

  Derived() : value(123) {}

  Box make(int tag)
  {
    Box out;
    out.p = (void *)(long)tag;
    return out;
  }

  bool check(int tag)
  {
    return tag == 77 && value == 123;
  }

  bool run()
  {
    return check(77);
  }
};

int main()
{
  Derived d;
  if(!d.run()) {
    return 1;
  }
  if(!d.run()) {
    return 2;
  }
  return 0;
}
