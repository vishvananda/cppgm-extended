// N3485 13.3.3p1 and 13.3.3.2p3-p4: rank second standard member-pointer conversions. expect exit 0
struct A { int m; }; struct B:A {}; struct C:B {}; int chosen;
int f(int B::*){return 1;} int f(int C::*){return 2;}
int main(){return f(&A::m)==1 ? 0:1;}
