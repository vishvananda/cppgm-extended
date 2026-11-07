// N3485 focus: 3.2 [basic.def.odr]; adopted CWG1472/1741/1926/2083/2170.
// expect exit 0
struct O { int x, y; }; int read(int O::*p) { constexpr O object = {2,5}; return [](int O::*q) { return object.*q; }(p); } int main() { return read(&O::x) + read(&O::y) != 7; }
