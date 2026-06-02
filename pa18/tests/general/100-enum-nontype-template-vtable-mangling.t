enum Policy
{
  P0,
  P1,
  P2
};

template<class T, Policy P>
struct Control
{
  virtual ~Control() {}

  virtual int value()
  {
    return P;
  }
};

int main()
{
  Control<int, P2> control;
  return control.value() == 2 ? 0 : 1;
}
