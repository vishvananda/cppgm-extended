// N3485 14.3/14.5.3; a known extra argument after an unresolved expansion still proves excess; expect reject
template<class T> struct Box {};
template<class... T> struct Pack {};
template<template<class...> class Form, class... T>
int inspect(Form<int, T..., long>*) { return sizeof...(T); }
int main() {
  Pack<int, char, long>* valid = 0;
  int count = inspect<Pack, char>(valid);
  return count + inspect<Box>(0);
}
