// N3485 7.1.5p9: a constexpr variable must still have literal type.
struct Count { int n; constexpr Count():n(1){} ~Count(){} };
struct Holder { int pad; Count count; constexpr Holder():pad(2),count(){} };
constexpr Holder holder;
int main(){ return holder.count.n; }
