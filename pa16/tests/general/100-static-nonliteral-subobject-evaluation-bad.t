// N3485 5.19p2: an initializer temporary must still have literal type.
struct Count { int n; constexpr Count():n(1){} ~Count(){} };
struct Holder { int n; constexpr Holder(int v):n(v){} };
constexpr Holder holder(Count().n);
int main(){ return holder.n; }
