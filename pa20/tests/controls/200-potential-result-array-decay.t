// N3485 focus: 3.2 [basic.def.odr]; adopted CWG1472/1741/1926/2083/2170.
// expect reject
struct O { int data[3]; }; int main() { constexpr O object = {{2,3,5}}; return []() { return object.data; }()[0]; }
