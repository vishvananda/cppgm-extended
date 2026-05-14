struct tag {
  char c;
  tag(char v = 'x') : c(v) {}
};

struct base {
  virtual ~base() {}
  virtual int f() const = 0;
};

struct mid : base {
  virtual int g() const { return 1; }
};

struct derived : tag, mid {
  derived() : tag('q') {}
  int f() const { return 2; }
  int g() const { return 7; }
};

int main()
{
  derived d;
  mid * p = &d;
  return p->g() == 7 ? 0 : 1;
}
