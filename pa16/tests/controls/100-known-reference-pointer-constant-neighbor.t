// expect exit 0
// N3485 5.19: known constant object reads and identities remain usable.
struct Value { int n; constexpr int get() const { return n; } }; constexpr Value object={7}; constexpr const Value& reference=object; constexpr const Value* pointer=&object;
static_assert(reference.get()==7 && pointer->get()==7 && &reference==pointer,"known object"); int main() { return 0; }
