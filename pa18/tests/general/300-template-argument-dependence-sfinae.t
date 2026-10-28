template<class...> struct Product {};
struct Identity {
    template<template<class...> class T, class... Ts>
    static auto make(Ts&&...) -> T<Ts...>;
};
template<template<class...> class T> struct Probe {
    template<class Id> static auto apply(int) -> decltype(Id::template make<T, int>());
    template<class> static char apply(...);
};
int main() {
    // The missing function argument discards the dependent candidate.
    return sizeof(decltype(Probe<Product>::apply<Identity>(0))) != sizeof(char);
}
