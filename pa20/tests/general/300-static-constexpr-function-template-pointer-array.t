// VALIDATION: run-pass
// N3485 focus: 14.3.2 [temp.arg.nontype], 9.4.2 [class.static.data]

typedef unsigned long size_t;

template<class = void>
struct table
{
  constexpr static size_t sizes[] = {13ul, 29ul};

  template<size_t Index, size_t Size = sizes[Index]>
  static size_t position(size_t hash)
  {
    return hash % Size;
  }

  constexpr static size_t (*positions[])(size_t) = {
    position<0, sizes[0]>,
    position<1, sizes[1]>
  };

  static size_t call(size_t hash, size_t index)
  {
    return positions[index](hash);
  }
};

template<class T>
constexpr size_t table<T>::sizes[];

template<class T>
constexpr size_t (*table<T>::positions[])(size_t);

int main()
{
  return table<>::call(42ul, 1ul) == 13ul ? 0 : 1;
}
