// Reduced from Boost.MP11 mp_filter. The dependent iota alias carries a
// non-type value through an intermediate sequence alias; reference collection
// must preserve that dependent alias instead of instantiating the sequence body
// in the wrong template scope.

typedef unsigned long size_t;

template<class T>
struct remove_const {
  typedef T type;
};

template<class T>
struct remove_const<const T> {
  typedef T type;
};

template<class T, T V>
struct constant {
  static const T value = V;
};

template<class... T>
struct list {
};

template<class L>
struct first_impl;

template<template<class...> class L, class T1, class... T>
struct first_impl<L<T1, T...> > {
  typedef T1 type;
};

template<class L>
using first = typename first_impl<L>::type;

template<class L>
struct size_impl;

template<template<class...> class L, class... T>
struct size_impl<L<T...> > {
  typedef constant<size_t, sizeof...(T)> type;
};

template<class L>
using size = typename size_impl<L>::type;

template<class T, T... I>
struct integer_sequence {
};

template<class T, T N>
using make_integer_sequence = __make_integer_seq<integer_sequence, T, N>;

template<class S, class F, bool Z>
struct from_sequence_impl;

template<template<class T, T... I> class S, class U, U... J, class F>
struct from_sequence_impl<S<U, J...>, F, true> {
  typedef list<constant<U, J>...> type;
};

template<class S, class F = constant<int, 0> >
using from_sequence =
    typename from_sequence_impl<S, F, (F::value == 0)>::type;

template<class N, class F = constant<int, 0> >
using iota =
    from_sequence<
        make_integer_sequence<
            typename remove_const<decltype(N::value)>::type,
            N::value>,
        F>;

template<size_t N>
using iota_c = from_sequence<make_integer_sequence<size_t, N> >;

template<bool C, class T, class E>
struct if_impl {
  typedef E type;
};

template<class T, class E>
struct if_impl<true, T, E> {
  typedef T type;
};

template<class C, class T, class E>
using if_ = typename if_impl<C::value, T, E>::type;

template<template<class...> class F, class... L>
struct transform_impl;

template<template<class...> class F,
         template<class...> class L1,
         class... T1,
         template<class...> class L2,
         class... T2>
struct transform_impl<F, L1<T1...>, L2<T2...> > {
  typedef L1<F<T1, T2>...> type;
};

template<template<class...> class F, class... L>
using transform = typename transform_impl<F, L...>::type;

template<template<class...> class P, class L1, class... L>
struct filter_impl {
  template<class T1, class... T>
  using f = if_<P<T1, T...>, list<T1>, list<> >;

  typedef transform<f, L1, L...> type;
};

template<class Q, class... L>
using filter_q = typename filter_impl<Q::template fn, L...>::type;

template<size_t N>
struct second_is {
  template<class T1, class T2>
  using fn = constant<bool, T2::value == N>;
};

template<class L, size_t N>
using at_c = first<filter_q<second_is<N>, L, iota<size<L> > > >;

typedef iota_c<3> input;
typedef at_c<input, 1> result;

int main()
{
  return 0;
}
