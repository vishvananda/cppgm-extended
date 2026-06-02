// VALIDATION: run-pass
// N3485 focus: 12.9 [class.inhctor], 14.8.2 [temp.deduct]
// A class-template specialization may inherit a constructor template from a
// dependent base. The synthesized inherited constructor must forward rvalue
// parameters to the selected base constructor template.

struct arg
{
  int value;

  explicit arg(int v)
    : value(v)
  {
  }
};

template <typename T>
struct base
{
  int value;

  template <typename U>
  explicit base(U&& u)
    : value(static_cast<U&&>(u).value)
  {
  }
};

template <typename T>
struct derived : base<T>
{
  using base<T>::base;
};

int main()
{
  arg a(7);
  derived<int> d(static_cast<arg&&>(a));
  return d.value == 7 ? 0 : 1;
}
