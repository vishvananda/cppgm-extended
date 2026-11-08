// expect exit 0
// Expanding a class template argument pack names one friend specialization.
template<class...> class T;
template<class... Ts> struct S {
  static int read() { return T<Ts...>::value; }
};
template<class... Ts> class T {
  friend class S<Ts...>;
  enum { value = sizeof...(Ts) };
};
int main() {
  return S<>::read() != 0 || S<int>::read() != 1 || S<int, char>::read() != 2;
}
