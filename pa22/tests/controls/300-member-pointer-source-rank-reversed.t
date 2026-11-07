// N3485 13.3.3p1 and 13.3.3.2p3-p4: rank second standard member-pointer conversions. expect exit 0
struct A { int m; }; struct B:A {}; struct C:B {}; int chosen;
typedef int A::* PA; typedef int B::* PB;
struct X { operator PB(){chosen=2;return &B::m;} operator PA(){chosen=1;return &A::m;} };
void f(int C::*p) { C c; c.m=8; if(c.*p!=8) chosen=0; }
int main(){X x; f(x); return chosen==2 ? 0:1;}
