// N3485 7.3.3p7-p8; P1099R5 relaxations start in C++20. expect reject
struct B{enum class E{value=7};};struct D:B{using B::E::value;};int main(){}
