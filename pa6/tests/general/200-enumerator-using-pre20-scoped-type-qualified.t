// N3485 7.3.3p7-p8; P1099R5 relaxations start in C++20. expect exit 0
namespace N{enum class E{value=7};}using N::E;using Alias=N::E;constexpr Alias value=E::value;static_assert(value==N::E::value,"");int main(){return value!=E::value;}
