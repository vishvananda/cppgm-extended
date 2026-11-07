// N3485 14.7.3p15: enum-specialization target eligibility.
// expect exit 0
template<class T>struct A{template<class U>struct B{enum class E;};}; template<>template<>enum class A<int>::B<char>::E{zero}; int main(){return int(A<int>::B<char>::E::zero);}
