// N3485 14.1p15, 14.3.3p3: the inner parameter-pack consumes Types...;
// its expanded head constrains each template argument, not the outer count.
template<class... Types> struct Outer {
  template<template<Types...> class... Templates>
  struct Member { static const int count = sizeof...(Templates); };
};
template<int, char> struct First {};
template<int, char> struct Second {};
typedef Outer<int, char>::Member<First, Second, First> Result;
int main() { return Result::count != 3; }
