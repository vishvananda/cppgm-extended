// expect exit 0
// GCC auto46.C; nested template-template parameter heads retain the function parameter's trailing return.
template<template<auto Fn() -> int> class> struct One {};
template<template<template<auto Fn() -> int> class> class> struct Two {};
int main() { return 0; }
