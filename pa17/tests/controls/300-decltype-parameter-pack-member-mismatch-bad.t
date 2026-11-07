// N3485 7.1.6.2 and 14.5.3: function parameter packs remain dependencies in decltype.
// expect reject
template<class... T> struct Tuple {};
template<class... Outer> struct A {
  template<class... Inner> static int f(Inner... args) {
    Tuple<decltype((int)args + (int)sizeof(Outer))...> t;
    return sizeof(t);
  }
};
int valid = A<char, long>::f(1, 2.0);
int bad = A<char, long>::f(1);
