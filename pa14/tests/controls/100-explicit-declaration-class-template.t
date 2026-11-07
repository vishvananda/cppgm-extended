// N3485 7.1.2p6: explicit appears only on constructor/conversion declarations inside their class definition.
// expect exit 0
template<class T> struct S { T x; explicit S(T n) : x(n) {} explicit operator bool() const { return x == 3; } }; int main() { S<int> s(3); return s ? 0 : 1; }
