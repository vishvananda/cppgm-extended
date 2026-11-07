// Adopted CWG2397: auto may occur inside a pointer/reference to an array; deduction type itself is not an array.
// expect reject
int main() { auto (*p)[2]; return 0; }
