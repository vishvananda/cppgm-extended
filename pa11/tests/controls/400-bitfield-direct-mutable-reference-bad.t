// expect reject
// I13: N3485 8.5.3: no direct nonconst lvalue-reference binding to a bit-field.
struct Bits { int value:4; }; int main() { Bits b={1}; int& reference=b.value; }
