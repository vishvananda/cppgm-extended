int values[] = {101, 103};
using index_type = long;
template<class T, class U = void> struct Table;
template<class T> struct Format : Table<T> {
  static constexpr int get(int n);
  constexpr int method(int n) const;
};
template<class U> struct Table<int, U> {
  using index_type = int;
protected:
  static constexpr int values[] = {3, 5};
};
template<class U> constexpr int Table<int, U>::values[];
template<class U> struct Table<char, U> {
  using index_type = int;
protected:
  static constexpr int values[] = {11, 13};
};
template<class U> constexpr int Table<char, U>::values[];
template<> constexpr int Format<int>::get(index_type n) {
  return (void)values[0], values[n];
}
template<> constexpr int Format<char>::get(index_type n) {
  return (void)values[0], values[n];
}
template<> constexpr int Format<int>::method(index_type n) const {
  return values[n];
}
constexpr Format<int> object{};
static_assert(Format<int>::get(1) == 5 && Format<char>::get(0) == 11, "concrete inherited static table");
static_assert(object.method(1) == 5, "ordinary member specialization");
int main() { return Format<int>::get(0) != 3 || Format<char>::get(1) != 13 || object.method(0) != 3; }
