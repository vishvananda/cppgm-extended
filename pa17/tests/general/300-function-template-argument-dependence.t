template<class...> struct Product {};
template<class... Ts> using Alias = Product<Ts...>;
struct Identity {
    template<template<class...> class T> static T<int> make();
    template<template<class...> class... Ts> static int pack();
};
template<template<class...> class T> struct Probe {
    template<class Id> static auto apply(Id) -> decltype(Id::template make<T>());
    template<class Id> static auto alias(Id) -> decltype(Id::template make<Alias>());
};
template<template<class...> class... Ts> struct PackProbe {
    template<class Id> static auto apply(Id) -> decltype(Id::template pack<Ts...>());
};
namespace functions {
    template<template<class...> class T> T<int> make();
}
template<class Outer> struct ScopeProbe {
    template<class Id> static auto apply(Id) -> decltype(functions::make<Product>());
};
int main() {
    return sizeof(decltype(Probe<Product>::apply(Identity{}))) != sizeof(Product<int>)
        || sizeof(decltype(Probe<Product>::alias(Identity{}))) != sizeof(Product<int>)
        || sizeof(decltype(PackProbe<Product, Alias>::apply(Identity{}))) != sizeof(int)
        || sizeof(decltype(ScopeProbe<int>::apply(0))) != sizeof(Product<int>);
}
