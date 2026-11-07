// N3485 14.7.3p1/p4/p15: enum-specialization target eligibility.
// expect exit 0
template<class T>struct A{struct B{enum class E;};}; template<>enum class A<int>::B::E{zero}; int main(){return int(A<int>::B::E::zero);}
