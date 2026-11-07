// N3485 7.1.6.2 and 14.5.3: function parameter packs remain dependencies in decltype.
// expect exit 0
template<class... T> struct Tuple {};
template<class... A> int sum(A...) { return 0; }
int pick(Tuple<int, int>*) { return 0; }
template<class... A> int f(A... args) {
  Tuple<decltype(sum(args...) + (int)args)...> t;
  return pick(&t);
}
int main() { return f(1, 2.0); }
