// N3485 7.3.3p7-p8; P1099R5 relaxations start in C++20. expect exit 0
struct B{enum E{value=7};};template<class T>struct D:T{using T::value;enum{width=value};};int main(){return D<B>::width!=7;}
