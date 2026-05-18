template<typename T>
struct SizeBase
{
  using size_type = unsigned long;
};

template<typename T>
struct InsertBase
{
  void insert(int) {}
  void insert(int, int) {}

  template<typename It>
  void insert(It, It) {}
};

template<typename T, bool Flag>
struct Insert;

template<typename T>
struct Insert<T, false> : InsertBase<T>
{
  using base_type = InsertBase<T>;
  using base_type::insert;

  template<typename Pair>
  void insert(Pair &&) {}

  template<typename Pair>
  void insert(int, Pair &&) {}
};

template<typename T>
struct Table : Insert<T, false>, SizeBase<T>
{
  using size_type = typename SizeBase<T>::size_type;

  template<typename It>
  Table(It first, It last, size_type hint);
};

template<typename T>
template<typename It>
Table<T>::Table(It first, It last, size_type hint)
{
  (void)hint;
  this->insert(first, last);
}

int main()
{
  int * p = 0;
  Table<int> t(p, p, 0);
  return 0;
}
