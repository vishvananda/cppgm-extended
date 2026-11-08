// expect reject
// N3485 14.3.3p3: corresponding parameters must have matching kinds.
template<template<class T, const T&> class Data> struct Main {};
template<class T, class U> struct Data {};
Main<Data> value;
