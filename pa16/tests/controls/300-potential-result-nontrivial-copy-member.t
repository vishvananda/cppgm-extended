// N3485 focus: 3.2 [basic.def.odr]; adopted CWG1472/1741/1926/2083/2170.
// expect exit 0
struct O { int x; constexpr O(int n) : x(n) {} constexpr O(const O& other) : x(other.x * 2) {} }; int main() { constexpr O object(17); struct Local { int read() { return object.x; } }; return Local().read() != 17; }
