// A genuinely dependent alias still requires the typename introducer.
template<class T> struct result {
  template<class... U> using fn = T;
};
template<class T> struct select { using type = result<T>; };
template<class T> using alias = typename select<T>::type;
template<class T> struct missing {
  using type = alias<T>::template fn<>;
};
int main() { return 0; }
