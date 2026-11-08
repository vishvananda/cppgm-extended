// expect reject
// P2280R4 deliberately excludes reading an ordinary unknown pointer value.
struct Value { constexpr int size() const { return 3; } };
void inspect(Value* value) { static_assert(value->size()==3,"unknown pointer"); }
int main() { return 0; }
