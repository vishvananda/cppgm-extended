// expect exit 0
// CWG 1495: a partial specialization must be more specialized than its primary.
template<int I, class T, class... Rest> struct A { static int result() { return 1; } };
template<class T, class... Rest> struct A<0, T, Rest...> { static int result() { return 2; } };
int main() { return A<0, int, short>::result() == 2 && A<1, int>::result() == 1 ? 0 : 1; }
