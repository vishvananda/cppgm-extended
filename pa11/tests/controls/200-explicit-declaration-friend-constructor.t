// N3485 7.1.2p6: explicit appears only on constructor/conversion declarations inside their class definition.
// expect reject
struct C { explicit C(int); }; struct S { explicit friend C::C(int); }; int main() { return 0; }
