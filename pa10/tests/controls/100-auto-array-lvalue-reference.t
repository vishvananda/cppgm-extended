// Adopted CWG2397: auto may occur inside a pointer/reference to an array; deduction type itself is not an array.
// expect exit 0
int values[2] = {3,5}; int main() { auto (&r)[2] = values; r[1] = 7; return values[1] - 7; }
