// Adopted CWG2397: auto may occur inside a pointer/reference to an array; deduction type itself is not an array.
// expect reject
int values[2]; int main() { auto (*p)[3] = &values; return 0; }
