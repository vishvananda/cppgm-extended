// VALIDATION: run-pass

template<class T>
struct store
{
  T value;
};

template<class... T>
struct data : store<T>...
{
};

template<class... T>
struct tuple : data<T...>
{
};

int main()
{
  typedef tuple<> empty;
  typedef tuple<empty> nested;
  static_assert(sizeof(nested) > 0, "empty base pack expands to no bases");
  return 0;
}
