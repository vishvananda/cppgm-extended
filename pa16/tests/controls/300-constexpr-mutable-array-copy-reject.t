// CWG 1405: an aggregate copy cannot bypass reads of mutable array members.
// expect reject. CXX11-019.
struct A { mutable int cache[2]; };
constexpr A source = {{7, 8}};
constexpr A copy = source;
int main() { return 0; }
