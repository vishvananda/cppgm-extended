// N3485 7.3.3p7-p8; P1099R5 relaxations start in C++20. expect reject
struct B{enum class E{value=7};};template<class T>struct D:T{using T::E::value;};D<B> d;int main(){}
