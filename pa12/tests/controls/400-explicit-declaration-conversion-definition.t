// N3485 7.1.2p6: explicit appears only on constructor/conversion declarations inside their class definition.
// expect exit 0
struct S { explicit operator bool() const; }; S::operator bool() const { return true; } int main() { S s; return s ? 0 : 1; }
