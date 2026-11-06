// N3485 8.5.1p8,12 and 14.8.2p8: invalid brace elision in the immediate
// context fails substitution; expect exit 0. CXX11-010 neighbor.
struct Empty {};
template<class T> auto probe(int) -> decltype(T{1}, char());
template<class T> long probe(...);
static_assert(sizeof(probe<Empty[2]>(0)) == sizeof(long), "empty element needs braces");
static_assert(sizeof(probe<int[2]>(0)) == sizeof(char), "ordinary array remains viable");
int main() { return 0; }
