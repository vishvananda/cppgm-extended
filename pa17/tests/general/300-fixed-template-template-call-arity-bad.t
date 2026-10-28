template<class...> struct Product {};
template<template<class...> class T> int make(int);
template<template<class...> class T> struct Probe {
    template<class Id> static auto apply(Id) -> decltype(make<T>());
};
// Once the outer template is bound, the call is fixed even though its
// surrounding member template is not instantiated.
Probe<Product> probe;
