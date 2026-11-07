// N3485 12.8p15-p16,5.19p2; GCC constexpr-empty13.C composition.
// expect exit 0
struct E {}; struct A { mutable E e; } a; constexpr A b=a; int main(){return 0;}
