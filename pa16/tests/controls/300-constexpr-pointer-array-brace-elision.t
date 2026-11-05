// N3485 5.19/8.5.1: constant evaluation follows aggregate brace elision.
// expect exit 0
struct Pointers { const char* values[3]; };
constexpr Pointers pointers = {"alpha", "beta", "gamma"};
static_assert(pointers.values[0][0] == 'a', "first pointer element");
static_assert(pointers.values[2][1] == 'a', "last pointer element");
int main() { return 0; }
