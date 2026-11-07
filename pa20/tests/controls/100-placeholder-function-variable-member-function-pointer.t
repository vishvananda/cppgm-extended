// CWG2476 initializer-based function return placeholder deduction; expect exit 0
template<class A,class B> struct Same { static const bool value=false; };
template<class A> struct Same<A,A> { static const bool value=true; };
struct C{int target(int n)const{return n+5;}};
int main(){auto (C::*p)(int) const=&C::target; C c;int result=(c.*p)(3)-8;return result;}
