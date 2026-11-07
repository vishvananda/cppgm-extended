// N3485 focus: 3.2 [basic.def.odr]; adopted CWG1472/1741/1926/2083/2170.
// expect exit 0
struct O { int data[3]; }; int main() { constexpr O object = {{2,3,5}}; int n = 0; auto commuted = [](int& index) { return (index++)[object.data]; }; auto direct = [](int index) { return object.data[index]; }; int first = commuted(n); int second = commuted(n); return n == 2 && first + second + direct(2) == 10 ? 0 : 1; }
