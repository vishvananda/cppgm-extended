// N3485 12.8p15-p16,5.19p2: an empty mutable member has no stored value to read.
// expect exit 0
struct E{};int n;struct A{mutable E e;int&r;};constexpr A f(const A&a){return a;}constexpr A b=f(A{{},n});static_assert(&b.r==&n,"");int main(){return 0;}
