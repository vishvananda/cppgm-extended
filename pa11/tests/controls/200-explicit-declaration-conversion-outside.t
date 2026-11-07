// N3485 7.1.2p6: explicit appears only on constructor/conversion declarations inside their class definition.
// expect reject
struct S { explicit operator bool() const; }; explicit S::operator bool() const { return true; } int main() { return 0; }
