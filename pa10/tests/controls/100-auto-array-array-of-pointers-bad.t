// Adopted CWG2397: auto may occur inside a pointer/reference to an array; deduction type itself is not an array.
// expect reject
int value = 5; int main() { auto* values[2] = {&value,&value}; return 0; }
