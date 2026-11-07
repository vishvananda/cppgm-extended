// N3485 focus: 3.2/5.1.2p17, adopted CWG2083/P0588R1: potential reference captures; body read retains original.
// expect exit 0
int copies = 0; int bump(int n) { ++copies; return n * 2; } constexpr int copy_value(int n) { return n == 0 ? 0 : bump(n); } struct O { int x; constexpr O(int n) : x(n) {} constexpr O(const O& other) : x(copy_value(other.x)) {} }; int main() { constexpr O object(17); auto read = [=]() { return object.x; }; return copies >= 1 && read() == 17 ? 0 : 1; }
