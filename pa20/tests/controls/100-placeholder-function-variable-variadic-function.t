// CWG2476 initializer-based function return placeholder deduction; expect exit 0
template<class A,class B> struct Same { static const bool value=false; };
template<class A> struct Same<A,A> { static const bool value=true; };
int target(int n,...){return n+2;}
int main(){auto (*p)(int,...)=target;static_assert(Same<decltype(p),int(*)(int,...)>::value,"variable type"); return p(5,8)-7;}

