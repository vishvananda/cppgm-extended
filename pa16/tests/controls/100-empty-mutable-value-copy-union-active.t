// N3485 12.8p15-p16,5.19p2: an empty mutable member has no stored value to read.
// expect exit 0
struct E{};union U{int x;unsigned y;};struct A{mutable E e;U u;};constexpr A f(const A&a){return a;}constexpr A b=f(A{{},U{7}});static_assert(b.u.x==7,"");int main(){return b.u.x!=7;}
