// An indexed shuffle has its inputs' type with one index per lane, and
// otherwise a vector of their kind with one lane per index.
typedef double D1 __attribute__((vector_size(8)));
typedef double D2 __attribute__((vector_size(16)));
typedef double D4 __attribute__((vector_size(32)));
typedef double D8 __attribute__((vector_size(64)));
typedef double E2 __attribute__((ext_vector_type(2)));
typedef double E4 __attribute__((ext_vector_type(4)));
typedef long long Q2 __attribute__((vector_size(16)));
typedef long long Q4 __attribute__((vector_size(32)));
D2 d;
E2 e;
Q2 q;
static_assert(__is_same(decltype(__builtin_shufflevector(d, d, 1, 0)), D2), "one index per lane");
static_assert(__is_same(decltype(__builtin_shufflevector(e, e, 1, 0)), E2), "one index per extended lane");
static_assert(__is_same(decltype(__builtin_shufflevector(d, d, 0)), D1), "a narrower GNU vector");
static_assert(__is_same(decltype(__builtin_shufflevector(d, d, 0, 1, 2, 3)), D4), "a wider GNU vector");
static_assert(__is_same(decltype(__builtin_shufflevector(e, e, 0, 1, 2, 3)), E4), "a wider extended vector");
static_assert(!__is_same(D4, E4), "the two kinds differ");
static_assert(__is_same(decltype(__builtin_shufflevector(q, q, 0, 1, -1, -1)), Q4), "unspecified lanes");
D8 widen(D2 x, D4 y) { return __builtin_shufflevector(__builtin_shufflevector(x, x, 0, 1, 2, 3), y, 0, 1, 2, 3, 4, 5, 6, 7); }
int main() { return 0; }
