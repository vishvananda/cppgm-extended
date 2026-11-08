struct AllocTag {};
template<bool B, class... T> struct Enable {};
template<class... T> struct Enable<true, T...> { using type = int; };
template<class... T> struct Tuple {
  enum { count = sizeof...(T) };
  int kind;
  Tuple(const T&...): kind(1) {}
  template<class A, bool NonEmpty = (sizeof...(T) > 0),
           typename Enable<NonEmpty, const T&...>::type = 0>
  Tuple(AllocTag, const A&, const T&...): kind(2) {}
};
template<class... T> Tuple(T...) -> Tuple<T...>;
template<class A, class... T> Tuple(AllocTag, const A&, T...) -> Tuple<T...>;
int main() {
  Tuple plain(1, 2);
  Tuple allocated(AllocTag{}, 0, 3);
  return plain.kind != 1 || plain.count != 2 || allocated.kind != 2 || allocated.count != 1;
}
