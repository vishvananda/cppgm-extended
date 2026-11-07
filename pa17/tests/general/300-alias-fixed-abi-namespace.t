// N3485 14.5.7p2/14.5.6.1; transparent alias specialization ABI, producer TU.
namespace N {
template<class A, class... B> struct List { int value; };
template<class A, class... B> using Alias = List<A, B...>;
template<class... T> int inspect(Alias<T...> value) { return value.value + sizeof...(T); }
template int inspect<int, char>(Alias<int, char>);
}
