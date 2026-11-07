// Reduced from Clang SemaCXX/GH63151.cpp; N3485 8.5.4p3/p7.
// A constructor list argument may not narrow through a reference temporary.
// expect reject
struct A { unsigned value; A(const unsigned& x) : value(x) {} }; void use(int p) { A object{p}; } int main() { use(3); return 0; }
