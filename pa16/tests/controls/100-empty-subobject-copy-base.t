// N3485 12.8p15-p16,5.19p2; GCC constexpr-empty13.C composition.
// expect exit 0
struct E {}; struct A { E e; }; struct B:A { E e[2]; } b; constexpr B c=b; int main(){return 0;}
