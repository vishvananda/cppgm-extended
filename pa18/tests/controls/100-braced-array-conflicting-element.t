// expect reject
// CWG 1591: every non-braced element must deduce the same element type.
template<class T, int N> int count(const T (&)[N]) { return N; }
int main() { return count({1, 2.0}); }
