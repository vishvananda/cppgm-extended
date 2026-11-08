// expect exit 0
// P2280R4 adopted DR: implicit this may be unknown when no member value is read.
struct Array { int values[3]; constexpr int size() const { return 3; } };
struct Owner { Array inner; void inspect() { static_assert(inner.size()==3,"unknown this subobject"); } };
extern Array& external; void query() { static_assert(external.size()==3,"unknown namespace reference"); }
int main() { Owner owner; owner.inspect(); return 0; }
