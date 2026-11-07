// Nested alias expansion boundary: N3485 14.5.3/14.5.7 and CWG1558.
// expect exit 0
template<class...> struct Pack {};
template<class A, class... B> using Kept = Pack<A, Pack<A, B>...>;
template<class...> struct Always;
template<bool> struct Enable;
template<bool B> using Enabled = typename Enable<B>::type;
template<class...> struct Badge {
 template<class... T, Enabled<Always<Kept<T>...>::value> = 0> int f() { return 0; }
};
Badge<> object;
template<class...> struct Always { static const bool value = true; };
template<bool> struct Enable { using type = int; };
int main() { return object.f<int, char>(); }
