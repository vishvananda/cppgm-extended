// CWG2476 initializer-based function return placeholder deduction; expect exit 0
template<class A,class B> struct Same { static const bool value=false; };
template<class A> struct Same<A,A> { static const bool value=true; };
int target(long n){return int(n)+4;} int(*choose(int))(long){return target;} using FunctionPointer=int(*(*)(int))(long);
int main(){auto (*(*p)(int))(long)=choose;static_assert(Same<decltype(p),int(*(*)(int))(long)>::value,"nested function result");int result=p(1)(2)-6;return result;}
