template<bool B, class T, class... E> struct conditional {};
template<class T, class... E> struct conditional<true, T, E...> { using type = T; };
template<class T, class E> struct conditional<false, T, E> { using type = E; };
template<class B, class T, class... E> using select = typename conditional<B::value,T,E...>::type;
template<class...> struct same { static const bool value = true; };
template<class T> struct size { using type = T; };
template<class... T> struct list {};
template<template<class...> class F, class... L> struct transform_impl { using type = list<L...>; };
struct error {};
template<template<class...> class F, class... L>
using transform = typename select<same<typename size<L>::type...>, transform_impl<F,L...>,error>::type;
template<class Q, class... L> using transform_q = transform<Q::template fn, L...>;
template<template<class...> class P> struct wrapper {
    template<class A, class... T> using fn = P<A, T...>;
};
template<template<class...> class P, class L1, class... L> struct filter {
    using type = transform_q<wrapper<P>, L1, L...>;
};
template<class A, class B> struct exact { static const bool value = false; };
template<class A> struct exact<A, A> { static const bool value = true; };
static_assert(exact<filter<list, int>::type, list<int>>::value, "empty tail");
static_assert(exact<filter<list, int, char, long>::type, list<int, char, long>>::value, "expanded tail");
int main() { return 0; }
