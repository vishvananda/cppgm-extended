// N3485 14.5.7/14.5.3; concrete member specialization and nested alias expansion; expect exit 0
template<class T, class... Rest> struct Foo {
  template<class U, class... More> using Bar = Foo<U, More...>;
  using Normal = Foo<Rest...>;
  using Deferred = Bar<Rest...>;
};
template<class A, class B> struct Same { static const bool value = false; };
template<class A> struct Same<A, A> { static const bool value = true; };
int main() {
  Foo<void, int> object;
  return !(Same<Foo<void, int>::Normal, Foo<int>>::value &&
           Same<Foo<void, int>::Deferred, Foo<int>>::value &&
           Same<Foo<void, int>::Bar<long, char>, Foo<long, char>>::value);
}
