// GCC PR104008: an empty inner expansion cannot erase outer pack dependence.
// N3485 14.5.3/14.5.7; retain dependent alias arguments (CWG1558).
// expect exit 0
template<class...> struct Pack {};
template<class A, class... B> using Erased = Pack<Pack<A, B>...>;
template<class...> struct Always;
template<bool> struct Enable;
template<bool B> using Enabled = typename Enable<B>::type;
template<class...> struct Badge {
 template<class... T, Enabled<Always<Erased<T>...>::value>> Badge();
 Badge() {}
};
Badge<> object;
template<class...> struct Always { static const bool value = true; };
template<bool> struct Enable { using type = int; };
int main() { return 0; }
