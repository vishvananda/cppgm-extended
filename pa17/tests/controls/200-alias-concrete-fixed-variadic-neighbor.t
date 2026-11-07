// N3485 14.5.7p2; concrete alias specializations do not need alias-template identity; expect exit 0
template<class A, class... B> struct List {};
template<class A, class... B> using Tail = List<A, B...>;
template<class A, class B> struct Same { static const bool value = false; };
template<class A> struct Same<A, A> { static const bool value = true; };
int main() { return !(Same<Tail<int>, List<int>>::value && Same<Tail<int, char>, List<int, char>>::value); }
