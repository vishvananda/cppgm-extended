// N3485 13.3.3p1 and 13.3.3.2p3-p4: rank second standard member-pointer conversions. expect exit 0
struct A { int m; }; struct B:A {}; struct C:B {}; int chosen;
typedef int A::* PA; typedef int B::* PB;PA pa=&A::m;PB pb=&B::m;
struct X { operator PA&(){chosen=1;return pa;} operator PB&(){chosen=2;return pb;} };
int main(){int C::*p=X();return chosen==2 && p==&C::m ? 0:1;}
