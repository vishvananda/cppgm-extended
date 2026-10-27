// VALIDATION: compile-pass
// An unqualified name imported from a dependent base remains a value argument.
namespace detail {
  template<bool> struct Tag {};
  template<class T> struct Box { T value; };
}
template<bool V> struct Base { static const bool flag = V; };
template<bool V> struct Derived : Base<V> {
  typedef Base<V> Parent;
  using Parent::flag;
  int consume(detail::Tag<true>) { return 7; }
  int consume(detail::Tag<false>) { return 3; }
  int run() { return consume(detail::Tag<flag>()); }
};
// An explicitly introduced dependent type still supplies a type argument.
template<class T> struct TypeBase { typedef T type; };
template<class T> struct Typed : TypeBase<T> {
  using typename TypeBase<T>::type;
  detail::Box<type> box;
};
int main() {
  Derived<true> a;
  Derived<false> b;
  Typed<int> typed;
  typed.box.value = 5;
  return a.run() == 7 && b.run() == 3 && typed.box.value == 5 ? 0 : 1;
}
