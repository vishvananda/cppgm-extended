// expect exit 0
template<bool B,class T=void>struct enable_if{};
template<class T>struct enable_if<true,T>{typedef T type;};
template<class T>struct predicate{static const bool value=true;};
struct special{};
template<class T>struct tag{};
namespace nested_type {
template<class Tag,class F,class=void>struct select;
template<class U>struct select<tag<U>,void(special)>{enum{value=1};};
template<class Tag,class T>struct select<Tag,void(T),typename enable_if<predicate<T>::value>::type>{enum{value=2};};
static_assert(select<tag<int>,void(special)>::value==1,"nested synthesized type");
static_assert(select<tag<long>,void(int)>::value==2,"ordinary substitution");
}
namespace nested_template {
template<class Tag,class F,class=void>struct select;
template<template<class>class Box>struct select<Box<special>,void(special)>{enum{value=1};};
template<class Tag,class T>struct select<Tag,void(T),typename enable_if<predicate<T>::value>::type>{enum{value=2};};
static_assert(select<tag<special>,void(special)>::value==1,"nested synthesized template");
static_assert(select<tag<int>,void(int)>::value==2,"ordinary template argument");
}
int main(){return 0;}
