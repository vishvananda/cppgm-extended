// Adopted CWG2397: auto may occur inside a pointer/reference to an array; deduction type itself is not an array.
// expect exit 0
template<class T> int read(T (&values)[2]) { auto (*p)[2] = &values; return (*p)[1]; } int main() { int values[2] = {3,5}; return read(values) - 5; }
