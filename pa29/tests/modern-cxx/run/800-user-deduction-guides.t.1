template<class T> struct Box {
  T value;
  template<class U> explicit Box(U v): value(v) {}
};
template<class U> Box(U) -> Box<U>;
Box(long) -> Box<short>;
template<class A, class B> struct Pair {
  A first; B second;
  template<class X, class Y> Pair(X x, Y y): first(x), second(y) {}
};
template<class X, class Y> Pair(X, Y) -> Pair<X, Y>;
template<class T> struct Stable { T value; Stable(const T& v): value(v) {} };
int before() { Stable value(1); return sizeof(value.value) == sizeof(int); }
template<class T> Stable(T) -> Stable<long long>;
template<class T> int dependent(T value) { Box result(value); return result.value; }
template<class T> struct Copy { T value; Copy(T v): value(v) {} };
template<class T> explicit Copy(T) -> Copy<long long>;
template<class F> struct Scope {
  F action;
  template<class G> Scope(G&& f): action(static_cast<G&&>(f)) {}
  ~Scope() { action(); }
};
template<class G> Scope(G) -> Scope<G>;
template<class T = int> struct Default { T value; Default(T v = 4): value(v) {} };
template<class T = long long> Default() -> Default<T>;
int main() {
  Box one(7);
  Box two(8L);
  Pair pair(4, 2.5);
  Stable later(3);
  Copy direct(2);
  Copy copied = 2;
  int calls = 0;
  { Scope finish([&] { calls += 7; }); }
  Default defaults;
  if (sizeof(direct.value) != sizeof(long long) || sizeof(copied.value) != sizeof(int) ||
      calls != 7 || defaults.value != 4 || sizeof(defaults.value) != sizeof(long long)) return 1;
  return one.value != 7 || sizeof(two.value) != sizeof(short) ||
    pair.first != 4 || pair.second != 2.5 || !before() ||
    sizeof(later.value) != sizeof(long long) || dependent(9) != 9;
}
