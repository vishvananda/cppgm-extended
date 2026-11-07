// N3485 focus: 3.2 [basic.def.odr]; adopted CWG1472/1741/1926/2083/2170.
// expect exit 0
int global = 3; int main() { const int& reference = global; struct Local { int read() { return reference; } const int* address() { return &reference; } }; Local local; int first = local.read(); global = 7; return first == 3 && local.read() == 7 && local.address() == &global ? 0 : 1; }
