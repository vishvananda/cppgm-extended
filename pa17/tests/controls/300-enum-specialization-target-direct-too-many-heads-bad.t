// N3485 14.7.3p15: enum-specialization target eligibility.
// expect reject
template<class T>struct A{enum class E;}; template<>template<>enum class A<int>::E{zero}; int main(){}
