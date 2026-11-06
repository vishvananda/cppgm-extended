// CWG 1405 (April 2013): reading a mutable member of an already
// initialized constexpr object is not a constant expression. expect reject
// CXX11-019; retain the original N3485 classification separately.
struct A { int fixed; mutable int cache; };
constexpr A object = {7, 8};
constexpr int value = object.cache;
int main() { return 0; }
