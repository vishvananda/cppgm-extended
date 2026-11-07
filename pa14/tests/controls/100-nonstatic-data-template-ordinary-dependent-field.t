// N3485 focus: 14 [temp], 9.2/9.4; PA17 documented variable-template extension.
// expect exit 0
template<class T> struct S { T member; };
int main() { S<int> a; S<int> b; a.member = 3; b.member = 7; return a.member == 3 && b.member == 7 ? 0 : 1; }
