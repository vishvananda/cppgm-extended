// N3485 focus: 3.2 [basic.def.odr]; adopted CWG1472/1741/1926/2083/2170.
// expect exit 0
int main() { int object = 17; struct Local { int read() { object; (void)object; return (object, 3); } }; return Local().read() != 3; }
