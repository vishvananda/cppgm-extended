// expect reject
// CWG2126 does not make mutable subobjects readable in a core constant expression.
struct Value { mutable int n; constexpr Value(int x):n(x) {} }; constexpr const Value& reference=Value(7);
static_assert(reference.n==7,"mutable is not constant storage"); int main() { return 0; }
