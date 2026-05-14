// VALIDATION: run-pass
// N3485 focus: 12.8 [class.copy]

struct Inner
{
  Inner() : value(new int(7)) {}
  ~Inner() { delete value; }

  Inner(const Inner &) = delete;
  Inner & operator=(const Inner &) = delete;

  Inner(Inner && other) : value(other.value)
  {
    other.value = 0;
  }

  Inner & operator=(Inner && other)
  {
    if(this != &other) {
      delete value;
      value = other.value;
      other.value = 0;
    }
    return *this;
  }

  int * value;
};

struct Outer
{
  Inner inner;
};

int main()
{
  Outer source;
  Outer moved(static_cast<Outer &&>(source));
  return *moved.inner.value == 7 ? 0 : 1;
}
