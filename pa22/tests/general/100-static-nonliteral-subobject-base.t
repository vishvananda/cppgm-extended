// N3485 3.6.2p2: nonliteral bases at nonzero offsets initialize statically.
struct First { int pad; constexpr First():pad(2){} };
struct Count { int n; constexpr Count():n(1){} ~Count(){} };
struct Holder:First,Count { constexpr Holder():First(),Count(){} };
extern Holder holder;
int observed;
struct Early { Early(){ observed=holder.n; holder.n=7; } } early;
Holder holder;
int main(){ return observed!=1 || holder.pad!=2 || holder.n!=7; }
