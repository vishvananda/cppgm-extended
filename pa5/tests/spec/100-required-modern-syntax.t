namespace course::syntax {
template<class T> struct Box { Box(T); };
Box(int) -> Box<long>;
template<auto N> struct Constant {};
template<template<class> typename C> struct Apply {};
template<class... T> int sum(T... values) { return (0 + ... + values); }
template<class... Bases> struct Import : Bases... { using Bases::choose...; };
struct Record {
  int member;
  auto closure() {
    return [*this, value(member)](auto argument) constexpr {
      if constexpr (sizeof(argument) > 0) return value + member + argument;
      else return 0;
    };
  }
};
decltype(auto) reference(int& value) { return (value); }
int selection(int input) {
  if (int value = input; value > 0) return value;
  switch (int value = input; value) { case 0: return 1; default: return 2; }
}
int binding() {
  int values[2] = {3, 7};
  auto& [first, second] = values;
  return first + second;
}
}
