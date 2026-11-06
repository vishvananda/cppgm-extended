// CWG 1405: taking a mutable member's constant address does not permit
// reading its value through that pointer. expect reject
// CXX11-019.
struct A { mutable int cache; };
constexpr A source = {8};
constexpr int* address = &source.cache;
constexpr int value = *address;
int main() { return 0; }
