// N3485 7.1.2p6: explicit appears only on constructor/conversion declarations inside their class definition.
// expect reject
struct S { template<class T> explicit int f(T n) { return int(n); } }; int main() { S s; return s.f(0); }
