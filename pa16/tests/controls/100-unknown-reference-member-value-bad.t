// expect reject
// P2280R4 preserves the prohibition on reading unknown object values.
struct Value { int n; constexpr int get() const { return n; } };
void inspect(const Value& value) { static_assert(value.get()==3,"unknown value"); }
int main() { return 0; }
