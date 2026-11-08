template<class T> struct Box { static const int value = sizeof(T); };
template<class T> using Alias = Box<T>;
template<template<class> typename C> struct Apply { using type = C<int>; };
template<template<class> typename C = Box> struct Default { using type = C<char>; };
template<template<class> typename... Cs> struct Pack { static const int count = sizeof...(Cs); };
static_assert(Apply<Box>::type::value == sizeof(int), "class template");
static_assert(Apply<Alias>::type::value == sizeof(int), "alias template");
static_assert(Default<>::type::value == 1, "default template");
static_assert(Pack<Box, Alias>::count == 2, "template pack");
