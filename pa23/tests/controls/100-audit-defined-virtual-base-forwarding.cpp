// AUDIT-ID: VBASE
// AUDIT-EXPECT: run
// The most-derived constructor initializes every scalar virtual base.
struct A { int a; A(int n=0):a(n){} };
struct B { int b; B(int n=0):b(n){} };
struct C { int c; C(int n=0):c(n){} };
struct D:virtual A,virtual B,virtual C {};
template<class E> struct W:E {
 W(const E& value,const int& location)
  :A(location),B(value.b),C(value.c),E(value){}
};
template<class E> W<E> make_wrap(const E& value,const int& location){return W<E>(value,location);}
int main(){D value;value.a=7;value.b=8;value.c=9;
 W<D> result=make_wrap(value,value.a);
 return result.a!=7||result.b!=8||result.c!=9;}
