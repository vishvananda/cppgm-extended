// N3485 12.8p15-p16 and 5.19p2: an empty trivial copy reads no member values.
// Reduced from GCC constexpr-ctor20.C and constexpr-empty12.C.
// expect reject
struct E{};void f(E& e){constexpr E c=e;}int main(){return 0;}
