// N3485 3.6.2p2: recursively initialized nonliteral members remain constant.
struct Count { int n; constexpr Count():n(1){} ~Count(){} };
struct Inner { int pad; Count count; constexpr Inner():pad(2),count(){} };
struct Holder { int pad; Inner inner; constexpr Holder():pad(3),inner(){} };
extern Holder holder;
int observed;
struct Early { Early(){ observed=holder.inner.count.n; holder.inner.count.n=7; } } early;
Holder holder;
int main(){ return observed!=1 || holder.pad!=3 || holder.inner.pad!=2 || holder.inner.count.n!=7; }
