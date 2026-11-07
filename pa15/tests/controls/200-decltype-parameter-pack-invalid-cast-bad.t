// N3485 7.1.6.2 and 14.5.3: function parameter packs remain dependencies in decltype.
// expect reject
template<class... T> struct Tuple {};
struct Bad {};
template<class... A> int f(A... args) {
  Tuple<decltype((int)args)...> t;
  return sizeof(t);
}
int valid = f(1);
int bad = f(Bad());
