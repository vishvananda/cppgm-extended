// CWG2476 initializer-based function return placeholder deduction; expect exit 0
template<class A,class B> struct Same { static const bool value=false; };
template<class A> struct Same<A,A> { static const bool value=true; };
int target(int n){return n+2;} long target(long n){return n+9;}
int main(){auto p=new (auto (*)(int))(target);using Allocated=int(*)(int); static_assert(Same<decltype(*p),Allocated&>::value,"allocated type"); int result=(*p)(5)-7; delete p; return result;}

