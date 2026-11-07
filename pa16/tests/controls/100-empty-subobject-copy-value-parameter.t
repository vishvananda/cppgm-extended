// N3485 12.8p15-p16,5.19p2; GCC constexpr-empty13.C composition.
// expect exit 0
struct E {}; struct A { E e; }; constexpr int f(A a){return (A(a),7);} static_assert(f(A{})==7,""); int main(){return 0;}
