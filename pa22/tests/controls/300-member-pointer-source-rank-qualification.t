// N3485 13.3.3p1 and 13.3.3.2p3-p4: rank second standard member-pointer conversions. expect exit 0
struct A { int m; }; struct B:A {}; struct C:B {}; int chosen;
typedef const int A::* PA; typedef int B::* PB;
struct X { operator PA(){chosen=1;return &A::m;} operator PB(){chosen=2;return &B::m;} };
int main(){const int C::*p=X();return chosen==2 ? 0:1;}
