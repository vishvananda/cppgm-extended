// expect exit 0
// N3485 14.1p8, 14.3.2p5, 14.3.3p3: typed template parameter normalization.
template<int Id, class Value, template<class T, const T&> class Data> struct Main {
  static Value value;
  template<template<class T, const T&> class Other> using Make = Other<Value, value>;
  using Result = Make<Data>;
};
template<int Id, class Value, template<class T, const T&> class Data> Value Main<Id, Value, Data>::value;
template<class T, const T&> struct Data {};
struct Value {};
int main() { Main<0, Value, Data> value; return 0; }
