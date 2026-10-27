// expect reject
// CWG 1591: different nonempty lists cannot deduce different values for N.
template<class T, int N> int same(const T (&)[N], const T (&)[N]) { return N; }
int main() { return same({1, 2}, {3}); }
