// N3485 3.6.2p2: array elements are subobjects of the initialized variable.
struct Count { int n; constexpr Count(int v):n(v){} ~Count(){} };
extern Count counts[2];
int observed;
struct Early { Early(){ observed=counts[0].n+counts[1].n; counts[1].n=7; } } early;
Count counts[2]={{1},{2}};
int main(){ return observed!=3 || counts[0].n!=1 || counts[1].n!=7; }
