// Written T function() remains a declaration; nonempty packs evaluate once and constructors run; expect exit 0
int function(){return 9;}template<class... A> int declaration(A...){typedef int T;T function();return function();}
int calls;int argument(){++calls;return 5;}struct X{int n;X():n(7){}};
template<class T,class... A>T make(A... a){T value(a...);return value;}
int main(){return declaration<>()!=9||make<X>().n!=7||make<int>(argument())!=5||calls!=1;}
