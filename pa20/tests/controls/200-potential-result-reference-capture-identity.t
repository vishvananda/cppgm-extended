// N3485 focus: 3.2 [basic.def.odr]; adopted CWG1472/1741/1926/2083/2170.
// expect exit 0
struct O { int x; }; int main() { constexpr O object = {17}; auto read = [&object]() { return &object.x; }; return read() == &object.x && *read() == 17 ? 0 : 1; }
