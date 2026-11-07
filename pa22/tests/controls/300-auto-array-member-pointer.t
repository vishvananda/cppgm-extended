// Adopted CWG2397: auto may occur inside a pointer/reference to an array; deduction type itself is not an array.
// expect exit 0
struct S { int values[2]; }; int main() { auto (S::*p)[2] = &S::values; S object = {{3,5}}; return (object.*p)[1] - 5; }
