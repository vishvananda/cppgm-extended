// N3485 14.1p15 and 14.5.3p5: Types in the first parameter controls
// the outer pack; the inner NTTP pack consumes the entire Types sequence.
template<class... Types> struct Outer {
  template<template<Types, Types...> class... Templates>
  struct Member { static const int count = sizeof...(Templates); };
};
template<int, int, char> struct IntFirst {};
template<char, int, char> struct CharFirst {};
typedef Outer<int, char>::Member<IntFirst, CharFirst> Result;
int main() { return Result::count != 2; }
