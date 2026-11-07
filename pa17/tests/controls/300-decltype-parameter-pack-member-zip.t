// N3485 7.1.6.2 and 14.5.3: function parameter packs remain dependencies in decltype.
// expect exit 0
template<class... T> struct Tuple {};
int pick(Tuple<int, int>*) { return 0; }
template<class... Outer> struct A {
  template<class... Inner> static int f(Inner... args) {
    Tuple<decltype((int)args + (int)sizeof(Outer))...> t;
    return pick(&t);
  }
};
int main() { return A<char, long>::f(1, 2.0); }
