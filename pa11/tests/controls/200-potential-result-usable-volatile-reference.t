// N3485 focus: 3.2 [basic.def.odr], adopted CWG1472/2083: usable reference value, actual volatile referent.
// expect exit 0
volatile int global = 3; int main() { const volatile int& reference = global; struct Local { int read() { return reference; } const volatile int* address() { return &reference; } }; Local local; int first = local.read(); global = 7; return first == 3 && local.read() == 7 && local.address() == &global ? 0 : 1; }
