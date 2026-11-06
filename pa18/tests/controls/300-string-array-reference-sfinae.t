// CWG 1490/1467, 14.8.2p8: list conversion failures in an immediate
// context discard candidates without poisoning later uses. expect exit 0
// CXX11-021; substitution owns this PA18:300 neighbor.
template<int N> int receive(const char (&)[N]);
template<int N> auto viable(int) -> decltype(receive<N>({"abc"}), char());
template<int> long viable(...);

static_assert(sizeof(viable<5>(0)) == sizeof(char), "padded array is viable");
static_assert(sizeof(viable<3>(0)) == sizeof(long), "literal exceeds bound");
static_assert(sizeof(viable<4>(0)) == sizeof(char), "exact bound binds literal");
static_assert(sizeof(viable<5>(0)) == sizeof(char), "failed candidate stays local");
int main() { return 0; }
