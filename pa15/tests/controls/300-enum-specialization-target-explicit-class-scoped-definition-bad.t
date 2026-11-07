// N3485 14.7.3p1/p4/p5: enum-specialization target eligibility.
// expect reject
template<class T>struct A; template<>struct A<int>{enum class E;}; template<>enum class A<int>::E{zero}; int main(){}
