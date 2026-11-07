// Adopted CWG2397: auto may occur inside a pointer/reference to an array; deduction type itself is not an array.
// expect exit 0
int values[2][3] = {{1,2,3},{5,7,11}}; int main() { auto (*p)[2][3] = &values; return (*p)[1][2] - 11; }
