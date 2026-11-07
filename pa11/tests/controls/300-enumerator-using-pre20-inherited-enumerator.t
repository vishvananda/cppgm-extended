// N3485 7.3.3p7-p8; P1099R5 relaxations start in C++20. expect exit 0
struct B{enum E{value=7};};struct D:B{using B::value;};struct F:D{using D::value;};int main(){return D::value!=7||F::value!=7;}
