// Adopted CWG2397: auto may occur inside a pointer/reference to an array; deduction type itself is not an array.
// expect exit 0
int values[2] = {3,5}; int main() { const auto (&r)[2] = values; return r[0] - 3; }
