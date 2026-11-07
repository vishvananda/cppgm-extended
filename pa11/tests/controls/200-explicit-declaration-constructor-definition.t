// N3485 7.1.2p6: explicit appears only on constructor/conversion declarations inside their class definition.
// expect exit 0
struct S { int x; explicit S(int); }; S::S(int n) : x(n) {} int main() { S s(3); return s.x - 3; }
