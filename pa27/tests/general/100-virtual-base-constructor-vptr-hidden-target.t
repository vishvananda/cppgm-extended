struct V {
  int tag;
  V() : tag(5) {}
  virtual int probe() { return 3; }
};

struct B : virtual V {
  B();
  virtual int anchor() { return 11; }
};

B::B() {}

struct Payload {
  int value;
  Payload() : value(7) {}
  virtual int read() { return 19; }
};

struct Holder {
  Payload payload;
  Holder() : payload() {}
};

struct D : Holder, B {
  D() : Holder(), B() {}
};

int main()
{
  D d;
  Payload *p = &d.payload;
  return p->read() == 19 ? 0 : 1;
}
