// N3485 14.5.7/14.5.3; scalar fixed argument plus a trailing pack; expect exit 0
template<class A, class... B> struct List {};
template<class A, class... B> using Tail = List<A, B...>;
template<class A, class... B> int inspect(Tail<A, B...>) { return sizeof...(B); }
int main() { return inspect<int, char, long>({}) - 2; }
