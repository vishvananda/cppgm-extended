// expect reject
// N3485 5.19: a pointer to a temporary destroyed at full-expression end cannot be published.
struct Value { int n; constexpr Value(int x):n(x) {} }; constexpr const Value* address(const Value& object) { return &object; }
constexpr const Value* escaped=address(Value(7)); int main() { return 0; }
