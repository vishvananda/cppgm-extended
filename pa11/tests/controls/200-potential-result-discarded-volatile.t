// N3485 focus: 3.2 [basic.def.odr]; adopted CWG1472/1741/1926/2083/2170.
// expect reject
int main() { volatile int object = 17; struct Local { int read() { (void)object; return 3; } }; return Local().read(); }
