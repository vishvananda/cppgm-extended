// Reduced from Boost.MP11 mp_similar_impl. Reusing the same template
// parameter pack in two template-id positions must reject candidates where the
// two deductions have different lengths.

template<bool B>
struct bool_constant
{
  static const bool value = B;
};

typedef bool_constant<true> true_type;
typedef bool_constant<false> false_type;

template<class... T>
struct list {};

template<class... T>
struct similar_impl;

template<class T1, class T2>
struct similar_impl<T1, T2>
{
  typedef false_type type;
};

template<template<class...> class L, class... T1, class... T2>
struct similar_impl<L<T1...>, L<T2...> >
{
  typedef true_type type;
};

template<template<class...> class L, class... T>
struct similar_impl<L<T...>, L<T...> >
{
  typedef false_type type;
};

static_assert(similar_impl<list<>, list<list<> > >::type::value,
              "same list template should use the independent-pack partial");

int main()
{
  return similar_impl<list<>, list<list<> > >::type::value ? 0 : 1;
}
