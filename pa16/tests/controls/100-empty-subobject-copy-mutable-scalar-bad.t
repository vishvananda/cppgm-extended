// N3485 12.8p15-p16,5.19p2; GCC constexpr-empty13.C composition.
// expect reject
struct E {}; struct A { E e; mutable int n; }; constexpr A a={{},1}; constexpr A b=a; int main(){return 0;}
