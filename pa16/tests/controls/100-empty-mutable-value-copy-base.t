// N3485 12.8p15-p16,5.19p2: an empty mutable member has no stored value to read.
// expect exit 0
struct E{};struct B{mutable E e;};struct A:B{};constexpr A f(const A&a){return a;}constexpr A b=f(A{});int main(){return 0;}
