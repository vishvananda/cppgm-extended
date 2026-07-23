// VALIDATION: compile-pass
namespace a { template<class> struct x { typedef char t; }; }
namespace b { template<class> struct x { typedef int t; }; }
template<template<class> class F> struct apply { typedef typename F<int>::t t; };
template<class> struct lambda;
template<template<class> class F, class T> struct lambda<F<T> > : apply<F> {};
static_assert(sizeof(lambda<a::x<int> >::t) == sizeof(char), "");
static_assert(sizeof(lambda<b::x<int> >::t) == sizeof(int), "");
int main() { return 0; }
