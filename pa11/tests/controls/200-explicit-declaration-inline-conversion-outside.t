// N3485 7.1.2p6: explicit appears only on constructor/conversion declarations inside their class definition.
// expect reject
struct S { explicit operator int() const; }; explicit inline S::operator int() const { return 1; } int main() { return 0; }
