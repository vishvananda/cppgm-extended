// N3485 13.3.3p1 and 13.3.3.2p3-p4: rank second standard member-pointer conversions. expect exit 0
struct A { int get(){return 9;} }; struct B:A {};struct C:B {};int chosen;
typedef int (A::*PA)();typedef int (B::*PB)();
struct X { operator PA(){chosen=1;return &A::get;} operator PB(){chosen=2;return &B::get;} };
int main(){int (C::*p)()=X();C c;return chosen==2 && (c.*p)()==9 ? 0:1;}
