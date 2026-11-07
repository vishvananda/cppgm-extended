// N3485 focus: 3.2 [basic.def.odr]; adopted CWG1472/1741/1926/2083/2170.
// expect reject
struct O { int x; mutable int y; }; int main() { constexpr O object = {17,3}; return []() { return object.x; }(); }
