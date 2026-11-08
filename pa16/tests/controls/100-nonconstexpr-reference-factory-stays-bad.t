// expect reject
// P2280R4 does not short-circuit evaluation of a non-constexpr reference-producing call.
struct Value { constexpr int size() const { return 3; } }; Value& factory();
void inspect() { static_assert(factory().size()==3,"call must be evaluated"); }
int main() { return 0; }
