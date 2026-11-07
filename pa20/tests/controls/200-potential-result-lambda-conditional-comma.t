// N3485 focus: 3.2 [basic.def.odr]; adopted CWG1472/1741/1926/2083/2170.
// expect exit 0
struct O { int x; }; int main() { constexpr O a = {17}; constexpr O b = {23}; int side = 0; auto read = [](bool flag, int& effects) { return (effects++, (flag ? a : b)).x; }; int first = read(true, side); int second = read(false, side); return first == 17 && second == 23 && side == 2 ? 0 : 1; }
