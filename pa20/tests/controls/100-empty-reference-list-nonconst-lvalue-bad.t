// expect reject
// I13: Adopted CWG2894 preserves ordinary nonconst lvalue-reference binding restrictions.
typedef int& Reference; int main() { Reference{}; }
