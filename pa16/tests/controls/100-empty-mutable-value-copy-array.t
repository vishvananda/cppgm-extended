// N3485 12.8p15-p16,5.19p2: an empty mutable member has no stored value to read.
// expect exit 0
struct E{};struct A{mutable E e[2][3];};constexpr A f(const A&a){return a;}constexpr A b=f(A{});int main(){return 0;}
