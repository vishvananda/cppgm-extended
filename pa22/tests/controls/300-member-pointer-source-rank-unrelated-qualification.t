// N3485 13.3.3p1 and 13.3.3.2p3-p4: rank second standard member-pointer conversions. expect reject
struct A{int m;};struct B{int n;};struct C:A,B{};
typedef const int A::*PA;typedef int B::*PB;struct X{operator PA();operator PB();};
void f(const int C::*);void test(X x){f(x);}
