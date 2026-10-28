// A specialization's member alias has no fixed parser class, but a concrete
// alias template-id is not dependent, even inside another template definition.
template<class A, class B> struct same { enum { value = 0 }; };
template<class A> struct same<A, A> { enum { value = 1 }; };
template<class T> struct result {
  template<class... U> using fn = T;
};
template<class T> struct select { using type = result<T>; };
template<class T> using alias = typename select<T>::type;
template<class T> using again = alias<T>;

static_assert(same<alias<int>::fn<>, int>::value, "empty alias arguments");
static_assert(same<alias<int>::fn<void>, int>::value, "nonempty alias arguments");
static_assert(same<again<short>::fn<char, int>, short>::value, "alias chain");

template<class Unused> int fixed() {
  return same<alias<int>::fn<>, int>::value;
}
template<class T> int dependent() {
  return same<typename alias<T>::template fn<>, T>::value;
}

struct values { enum { fn = 3 }; };
template<> struct select<long> { using type = values; };
struct functions {
  template<class... U> static constexpr int fn() { return 7; }
};
template<> struct select<char> { using type = functions; };
template<int N> struct integer { enum { value = N }; };
static_assert(integer<alias<char>::fn<>()>::value == 7, "function template expression");

int main() {
  return fixed<void>() != 1 || dependent<int>() != 1 ||
    !(alias<long>::fn < 4 || alias<long>::fn > 7);
}
