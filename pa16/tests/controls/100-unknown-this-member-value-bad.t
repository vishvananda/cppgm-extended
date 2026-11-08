// expect reject
// P2280R4 permits unknown this identity, not unknown member lvalue-to-rvalue conversion.
struct Value { int n; constexpr int get() const { return n; } void inspect() { static_assert(get()==3,"unknown value"); } };
int main() { return 0; }
