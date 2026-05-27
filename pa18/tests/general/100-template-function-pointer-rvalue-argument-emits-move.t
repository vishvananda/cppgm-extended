template<class T>
struct holder {
  T *px;

  holder() : px(0) {}
  holder(holder const &other) : px(other.px) {}
  holder(holder &&other) : px(other.px) { other.px = 0; }
  ~holder() {}
};

bool check(holder<int> value)
{
  return value.px != 0;
}

template<class T>
bool invoke(bool (*function)(T), T value)
{
  return function(static_cast<T &&>(value));
}

int main()
{
  int value = 1;
  holder<int> object;
  object.px = &value;
  return invoke(check, object) ? 0 : 1;
}

// VALIDATION: run-pass
