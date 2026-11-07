// N3485 focus:7.1.6/7.1.6.3,8.3.5,8.4.1,14.7.3:elaborated return-type use and template-name classification.
// expect exit 0
struct A{enum class E{zero};}; template<class T>int f(enum A::E){return 1;} template<>int f<int>(enum A::E e){return int(e);} int main(){return f<int>(A::E::zero);}
