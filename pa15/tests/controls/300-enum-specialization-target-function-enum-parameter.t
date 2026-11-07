// N3485 7.1.6.3p2,14.7.3p1: enum-specialization target eligibility.
// expect exit 0
struct A{enum class E{zero};}; template<class T>int f(enum A::E){return 1;} template<>int f<int>(enum A::E e){return int(e);} int main(){return f<int>(A::E::zero);}
