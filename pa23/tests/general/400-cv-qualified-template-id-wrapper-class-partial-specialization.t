// Reduced from Boost.Core type_name. A cv-qualified direct type-parameter
// partial specialization must beat a template-id partial specialization for a
// cv-qualified class-template-id argument.

template<class T>
struct holder {
  static const int value = 0;
};

template<class T>
struct holder<T const> {
  static const int value = 1;
};

template<template<class...> class L, class... T>
struct holder<L<T...> > {
  static const int value = 2;
};

template<class T, class U>
struct box {};

static_assert(holder<box<int, long> >::value == 2, "");
static_assert(holder<box<int, long> const>::value == 1, "");

int main()
{
  return holder<box<int, long> const>::value == 1 ? 0 : 1;
}
