// expect reject
// CWG1965 class/array materialization does not authorize scalar-prvalue const_cast references.
int main() { auto&& value=const_cast<int&&>(7); return value; }
