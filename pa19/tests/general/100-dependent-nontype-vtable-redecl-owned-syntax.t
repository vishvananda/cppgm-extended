template<class T, T V>
struct Control;

template<class U, U W>
struct Control
{
  virtual ~Control() {}

  virtual int value()
  {
    return W;
  }
};

int main()
{
  Control<int, 7> control;
  return control.value() == 7 ? 0 : 1;
}
