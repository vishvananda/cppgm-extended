// N3485 focus: 3.2 [basic.def.odr]; adopted CWG1472/1741/1926/2083/2170.
// expect exit 0
struct O { static constexpr int data[3] = {2,3,5}; }; int read(int n) { return O::data[n]; } int main() { return read(0) + read(1) + read(2) != 10; }
