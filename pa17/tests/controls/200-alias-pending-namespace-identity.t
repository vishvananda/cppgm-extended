// N3485 14.5.7p2/14.5.6.1; retained alias names preserve their declaration namespaces; expect exit 0
namespace A {
template<class X, class Y> struct Pair { int value; };
template<class X, class Y = char> using Alias = Pair<X, Y>;
}
namespace B {
template<class X, class Y> struct Pair { int value; };
template<class X, class Y = char> using Alias = Pair<X, Y>;
}
template<class... T> int read(A::Alias<T...> value) { return value.value + 3; }
template<class... T> int read(B::Alias<T...> value) { return value.value + 5; }
int main() {
  A::Pair<int, char> first = {7};
  B::Pair<int, char> second = {11};
  return read<int>(first) + read<int>(second) - 26;
}
