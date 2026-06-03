struct V {
  int value;
  virtual int anchor() { return 1; }
};

struct B : virtual V {
  long pad;
  virtual int banchor() { return 2; }
};

struct D : B {
  long more[8];
  virtual int danchor() { return 3; }
};

int read_value(B & b) { return b.value; }

struct Holder {
  B * p;
  Holder(B & b) : p(&b) {}
  int read() { return read_value(*p); }
};

int main()
{
  D d;
  d.value = 7;
  Holder h(d);
  return h.read() == 7 ? 0 : 1;
}
