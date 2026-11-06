// N3485 8.5.1p6, 14.8.2p8: excess array elements in an immediate
// context fail substitution; expect exit 0. CXX11-013.
template<class T> auto flat(int) -> decltype(T{1, 2}, char());
template<class T> long flat(...);
template<class T> auto nested(int) -> decltype(T{{1, 2}}, char());
template<class T> long nested(...);

static_assert(sizeof(flat<int[2]>(0)) == sizeof(char), "exact bound is viable");
static_assert(sizeof(flat<int[1]>(0)) == sizeof(long), "excess elements discard candidate");
static_assert(sizeof(flat<int[3]>(0)) == sizeof(char), "omitted tail remains viable");
static_assert(sizeof(flat<int[2]>(0)) == sizeof(char), "failure does not poison valid uses");
static_assert(sizeof(flat<int>(0)) == sizeof(long), "scalar neighbor fails substitution");
static_assert(sizeof(nested<int[1][1]>(0)) == sizeof(long), "nested excess elements discard");
static_assert(sizeof(nested<int[1][2]>(0)) == sizeof(char), "nested exact bound is viable");
int main() { return 0; }
