// N3485 focus: 14.1p15 / 14.3.3p3 / 14.5.3p4-p5 outer pack-expanded template heads
template<class... Types>
struct Outer {
  template<template<Types> class... Templates>
  struct Member { static const int count = sizeof...(Templates); };
};
template<int> struct IntHead {};
template<char> struct CharHead {};
Outer<int, char>::Member<IntHead> object;
int main() { return 0; }
