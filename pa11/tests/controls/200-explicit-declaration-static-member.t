// N3485 7.1.2p6: explicit appears only on constructor/conversion declarations inside their class definition.
// expect reject
struct S { static explicit void f(); }; int main() { return 0; }
