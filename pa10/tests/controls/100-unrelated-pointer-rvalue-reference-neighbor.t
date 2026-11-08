// expect exit 0
// A conversion to an unrelated pointer type still creates a temporary.
int main() { int n = 3; int* p = &n; const void*&& r = p; return r == p ? 0 : 1; }
