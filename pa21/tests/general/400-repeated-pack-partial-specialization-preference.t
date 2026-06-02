// Reduced from Boost.MP11 mp_similar_impl. When two partial
// specializations both match the same template-id pair, reusing the same pack
// in both positions is more specialized than using two independent packs.

template<class... T>
struct list {};

template<class T1, class T2>
struct similar_impl
{
  static const int value = 0;
};

template<template<class...> class L, class... T1, class... T2>
struct similar_impl<L<T1...>, L<T2...> >
{
  static const int value = 1;
};

template<template<class...> class L, class... T>
struct similar_impl<L<T...>, L<T...> >
{
  static const int value = 2;
};

static_assert(similar_impl<list<>, list<> >::value == 2,
              "same empty lists should use the repeated-pack partial");
static_assert(similar_impl<list<int>, list<int> >::value == 2,
              "same non-empty lists should use the repeated-pack partial");
static_assert(similar_impl<list<int>, list<long> >::value == 1,
              "different lists should use the independent-pack partial");

int main()
{
  return 0;
}
