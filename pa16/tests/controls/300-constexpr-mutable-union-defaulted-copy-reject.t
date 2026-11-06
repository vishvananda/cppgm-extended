// A defaulted union copy with a mutable variant is not constexpr in C++11.
// expect reject. CXX11-030; CWG2004, adopted as a DR in October 2015:
// https://cplusplus.github.io/CWG/issues/2004.html
union A { int fixed; mutable int cache; };
constexpr A source = {7};
constexpr A copy = source;
int main() { return 0; }
