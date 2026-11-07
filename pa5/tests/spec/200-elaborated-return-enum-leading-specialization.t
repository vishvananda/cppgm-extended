// N3485 focus:7.1.6/7.1.6.3,8.3.5,8.4.1,14.7.3:elaborated return-type use and template-name classification.
// expect exit 0
struct A{enum class E{zero};}; template<class T>enum A::E f(T){return A::E::zero;} template<>enum A::E f<int>(int){return A::E::zero;} int main(){return int(f(0));}
