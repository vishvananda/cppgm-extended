template<class...> struct list {};

template<class L, class T> struct append;

template<template<class...> class L, class... T, class U>
struct append<L<T...>, U> {
  typedef L<T..., U> type;
};

template<class L, class T>
using append_t = typename append<L, T>::type;

struct x1; struct x2; struct x3; struct x4; struct x5;
struct x6; struct x7; struct x8; struct x9; struct x10;

template<class A, class B> struct same_type { static const bool value = false; };
template<class A> struct same_type<A, A> { static const bool value = true; };

typedef append_t<append_t<append_t<append_t<append_t<
        append_t<append_t<append_t<append_t<append_t<
        list<>, x1>, x2>, x3>, x4>, x5>, x6>, x7>, x8>, x9>, x10> actual;

static_assert(same_type<actual,
                        list<x1, x2, x3, x4, x5,
                             x6, x7, x8, x9, x10> >::value, "");

int main() { return 0; }
