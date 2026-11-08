// N3485 14.3.3p1/p3 and 14.5.7p2: an alias can be a template argument without template identity equivalence; expect exit 0
template<template<class...> class T, class... Y> struct D {
  template<class... Z> using type = T<Y..., Z...>;
};
template<class T> struct A {};
template<class X, class Y> struct B;
template<class T> struct B<int, T> { using type = A<T>; };
template<class X, class Y> using C = typename B<X, Y>::type;
int identify(A<char>) { return 7; }
int main() { D<C>::type<int, char> value; return identify(value) - 7; }
