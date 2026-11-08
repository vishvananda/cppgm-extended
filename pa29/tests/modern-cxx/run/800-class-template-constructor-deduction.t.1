template<class T> struct Span {
  const T* pointer;
  unsigned long count;
  Span(const T* p, unsigned long n) : pointer(p), count(n) {}
  template<unsigned long N> Span(const T (&p)[N]) : pointer(p), count(N) {}
};
template<class T> struct Lock {
  const T* pointer;
  explicit Lock(const T& value) : pointer(&value) {}
};
template<class T> struct Pair {
  T first;
  long second;
  template<class U> Pair(T a, U b) : first(a), second(b) {}
};
template<class T> struct Box { T value; Box(T v) : value(v) {} };
template<class T = long> struct Default {
  T value;
  template<class U = T> Default(U v = U()) : value(v) {}
};
template<class T> struct Pack {
  int count;
  template<class... U> Pack(T, U...) : count(sizeof...(U)) {}
};
template<class T> int read(T& value) { Lock guard(value); return *guard.pointer; }
template<class... Args> unsigned long length(Args... args) {
  int values[sizeof...(args)] = { args... };
  return Span(values).count;
}
int main() {
  int values[3] = {4,5,6};
  auto array = Span(values);
  Span prefix(values, 2);
  const Lock guard(values[1]);
  Span braced{values};
  Pair pair(7, 11L);
  Box original(13); Box copy(original);
  Default chosen(17); Default zero;
  Pack packed(1, 2, 3L);
  return array.count != 3 || array.pointer[2] != 6 || prefix.count != 2 ||
    *guard.pointer != 5 || braced.count != 3 || pair.first != 7 || pair.second != 11 ||
    read(values[0]) != 4 || length(1,2) != 2 || copy.value != 13 ||
    chosen.value != 17 || zero.value != 0 || packed.count != 2;
}
