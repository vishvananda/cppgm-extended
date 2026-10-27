// expect reject
// CWG 1591: an empty list supplies no element type or array bound.
template<class T, int N> int count(const T (&)[N]) { return N; }
int main() { return count({}); }
