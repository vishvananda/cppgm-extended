// expect exit 0
// Adopted CWG2126: a const literal temporary extended by a usable reference is usable.
struct Value { const Value* self=this; int n=7; }; constexpr const Value& reference=Value{};
static_assert(reference.self==&reference && reference.n==7,"self and value belong to extended storage"); int main() { return reference.self!=&reference; }
