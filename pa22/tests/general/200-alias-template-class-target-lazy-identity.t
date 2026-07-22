template<class T> struct tuple {
  char storage[sizeof(T)];
};

template<class...> struct list {
};

template<class L1, class L2> struct assign;

template<template<class...> class L1, class... T,
         template<class...> class L2, class... U>
struct assign<L1<T...>, L2<U...> > {
  typedef L1<U...> type;
};

template<class L1, class L2>
using assign_t = typename assign<L1, L2>::type;

int main() {
  typedef assign_t<tuple<int>, list<int[]> > rebound;
  static_assert(__is_same(rebound, tuple<int[]>), "");
  return 0;
}
