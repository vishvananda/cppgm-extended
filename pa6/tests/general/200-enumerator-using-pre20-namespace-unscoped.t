// N3485 7.3.3p7-p8; P1099R5 relaxations start in C++20. expect exit 0
namespace N{enum E{value=7};}using N::E::value;using N::value;void f(){using N::E::value;static_assert(value==7,"");}static_assert(value==7,"");int main(){f();return value!=7;}
