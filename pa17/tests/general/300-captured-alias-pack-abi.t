// CXX11-124: captured alias packs and ordinary class packs have distinct ABI forms.
// Clang is the course mangled-name oracle (TESTING_AND_REFERENCES.md).
namespace captured {
template<int...> struct Values {};
template<class... T> using Counted = Values<(sizeof(T)+sizeof...(T))...>;
template<class... T> using Append = Counted<T..., char>;
template<class... T> int mixed(Append<T...>*) { return 7; }
template<class Unused,class... T> int indexed(Counted<T...>*) { return 11; }
template<class... T> struct Holder {
 template<class U> int member(Values<(sizeof...(T)+sizeof(U))>*) { return 13; }
};
}
int provide() {
 captured::Holder<int,char> h;
 return captured::mixed<int>(nullptr) + captured::indexed<int,char>(nullptr) + h.member<short>(nullptr);
}

template int captured::mixed<int>(captured::Append<int>*);
template int captured::indexed<int,char>(captured::Counted<char>*);
