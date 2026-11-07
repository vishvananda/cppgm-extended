// expect exit 0
// N3485 14.8.1p6,p9: fixed explicit prefix and deduced residual pack.
template<class>struct A{};template<class>struct B{};
template<template<class>class...T>struct Templates{};
template<template<class>class...T>int f(Templates<T...>){return sizeof...(T);}
int main(){Templates<A,B> t;return f<A>(t)==2&&f<A,B>(t)==2?0:1;}
