// Minimized from pa19/tests/general/141-dependent-qualified-return-type.t
template<unsigned long I, class... T>
struct tuple;

template<unsigned long I, class T>
struct tuple_element;

template<unsigned long I, class... T>
typename tuple_element<I, tuple<T...>>::type* g();
