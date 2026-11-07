// N3485 12.8p15-p16,5.19p2; GCC constexpr-empty13.C composition.
// expect exit 0
struct E {}; struct A { E e; } a; constexpr int f(A){return 42;} static_assert(f(a)==42,""); int main(){return 0;}
