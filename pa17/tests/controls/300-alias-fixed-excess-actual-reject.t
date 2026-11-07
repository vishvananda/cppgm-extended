// N3485 14.3/14.5.3/14.5.7; actual nonempty expansion has too many arguments; expect reject
template<class A, class B> struct Pair {};
template<class A, class B> using Both = Pair<A, B>;
template<class... T> struct Use { using Result = Both<T..., int>; };
Use<char, long> invalid;
int main() {}
