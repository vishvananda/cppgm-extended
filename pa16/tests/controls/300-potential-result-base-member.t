// N3485 focus: 3.2 [basic.def.odr]; adopted CWG1472/1741/1926/2083/2170.
// expect exit 0
struct Base { int a, b; }; struct Derived : Base { constexpr Derived(int a, int b) : Base{a,b} {} }; int main() { constexpr Derived object = {17,23}; struct Local { int read() { static_assert(object.a == 17, ""); return object.b; } }; return Local().read() != 23; }
