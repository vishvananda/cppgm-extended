// N3485 3.6.2p2: constant initialization includes nonliteral member objects.
struct Count { int n; constexpr Count():n(1){} ~Count(){} };
struct Holder { int pad; Count count; constexpr Holder():pad(2),count(){} };
extern Holder holder;
int observed;
struct Early { Early(){ observed=holder.count.n; holder.count.n=7; } } early;
Holder holder;
int main(){ return observed!=1 || holder.pad!=2 || holder.count.n!=7; }
