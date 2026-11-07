// N3485 12.8p15-p16 and 5.19p2: an empty trivial copy reads no member values.
// Reduced from GCC constexpr-ctor20.C and constexpr-empty12.C.
// expect exit 0
struct E{};struct H{E e;};H h;constexpr E b=h.e;int main(){return 0;}
