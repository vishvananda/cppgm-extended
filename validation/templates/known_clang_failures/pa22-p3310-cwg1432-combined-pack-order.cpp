// Expected to compile under the repository's N3485 C++11 contract.
// Clang 20+ rejects only the final assertion after LLVM PR 124137.

namespace combined_pack_order {

template<class>
struct select;

template<template<class, class...> class C, class A, class... Rest>
struct select<C<A, Rest...> > {
  static const int value = 1;
};

template<template<class> class C, class A>
struct select<C<A> > {
  static const int value = 2;
};

template<class>
struct unary;

static_assert(select<unary<int> >::value == 2,
              "combining both fixed-arity advantages must not reverse order");

} // namespace combined_pack_order

int main() {
  return 0;
}
