// N3485 12.8p15-p16,5.19p2: an empty mutable member has no stored value to read.
// expect exit 0
struct A{int n;};constexpr A f(const A&a){return a;}constexpr A b=f(A{7});static_assert(b.n==7,"");int main(){return b.n!=7;}
