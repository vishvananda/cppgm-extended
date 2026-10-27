// VALIDATION: compile-pass
// Member-template hiding depends on parameters, even before return substitution.
template<bool, class T> struct Enable {};
template<class T> struct Enable<true, T> { typedef T type; };
struct Config { typedef int type; };
template<class C> struct Base {
  template<class K>
  typename Enable<(sizeof(K)>0), typename C::type>::type erase(const K&) { return 1; }
  int erase(int, int) { return 3; }
};
template<class C> struct Before : Base<C> {
  using Base<C>::erase;
  typedef typename C::type size_type;
  template<class K>
  typename Enable<(sizeof(K)>0), size_type>::type erase(const K&) { return 2; }
};
template<class C> struct After : Base<C> {
  typedef typename C::type size_type;
  template<class K>
  typename Enable<(sizeof(K)>0), size_type>::type erase(const K&) { return 2; }
  using Base<C>::erase;
};
int main() {
  const int value=0;
  Before<Config> before;
  After<Config> after;
  return before.erase(value)==2 && after.erase(value)==2 &&
         before.erase(0,0)==3 && after.erase(0,0)==3 ? 0 : 1;
}
