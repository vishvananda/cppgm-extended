// expect exit 0
// I13: Adopted CWG1591: fixed element types permit list conversions while the bound is deduced from length.
template<int N> int a(const char (&v)[N]) { static_assert(N==3,"bound"); return v[0]+v[2]; }
template<int N> int b(const short (&v)[N]) { static_assert(N==3,"bound"); return v[0]+v[2]; }
template<int N> int c(const unsigned char (&v)[N]) { static_assert(N==3,"bound"); return v[0]+v[2]; }
template<int N> int d(const long (&v)[N]) { static_assert(N==3,"bound"); return v[0]+v[2]; }
int main() { return a({1,2,3})!=4 || b({1,2,3})!=4 || c({1,2,3})!=4 || d({1,2,3})!=4; }
