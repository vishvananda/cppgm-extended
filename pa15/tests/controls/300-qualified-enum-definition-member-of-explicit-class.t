// N3485 14.7.3p5 and 7.2p4: an enum member of an explicitly specialized class has an ordinary qualified definition.
// expect exit 0
template<class T>struct A; template<>struct A<int>{enum class E;}; enum class A<int>::E{zero}; int main(){return int(A<int>::E::zero);}
