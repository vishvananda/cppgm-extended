// CWG2476 initializer-based function return placeholder deduction; expect exit 0
template<class A,class B> struct Same { static const bool value=false; };
template<class A> struct Same<A,A> { static const bool value=true; };
long target(int n){return n+8;}
int main(){auto (*p)(int)->auto=target;static_assert(Same<decltype(p),long(*)(int)>::value,"variable type"); return int(p(2))-10;}

