// N3485 12.8p15-p16,5.19p2; GCC constexpr-empty13.C composition.
// expect exit 0
struct E {}; union U { E e; int n; constexpr U():n(42){} }; struct A { U u; }; constexpr A a={U{}}; constexpr A b=a; static_assert(b.u.n==42,""); int main(){return 0;}
