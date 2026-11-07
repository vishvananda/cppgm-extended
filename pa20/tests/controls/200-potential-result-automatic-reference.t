// N3485 focus: 3.2 [basic.def.odr]; adopted CWG1472/1741/1926/2083/2170.
// expect reject
int main() { int object = 17; int& reference = object; return []() { return reference; }(); }
