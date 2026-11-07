// Adopted CWG2397: auto may occur inside a pointer/reference to an array; deduction type itself is not an array.
// expect exit 0
int value = 5; int main() { auto* p = &value; return *p - 5; }
