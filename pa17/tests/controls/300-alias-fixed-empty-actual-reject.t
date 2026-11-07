// N3485 14.3/14.5.7; a real empty argument list cannot supply the fixed parameter; expect reject
template<class A> using Identity = A;
template<class... T> struct Use { using Result = Identity<T...>; };
Use<> invalid;
int main() {}
