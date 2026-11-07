// N3485 12.8p15-p16,5.19p2; GCC constexpr-empty13.C composition.
// expect reject
struct E {}; int n; struct A { E e; int &r; } a={{},n}; constexpr A b=a; int main(){return 0;}
