// N3485 3.6.2p2: a non-constexpr subobject constructor remains dynamic.
int calls;
struct Count { int n; Count():n(++calls){} ~Count(){} };
struct Holder { int pad; Count count; Holder():pad(2),count(){} };
extern Holder holder;
int observed;
struct Early { Early(){ observed=holder.count.n; holder.count.n=7; } } early;
Holder holder;
int main(){ return observed!=0 || holder.pad!=2 || holder.count.n!=1 || calls!=1; }
