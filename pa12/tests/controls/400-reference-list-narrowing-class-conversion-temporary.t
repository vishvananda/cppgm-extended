// Reduced from Clang SemaCXX/GH63151.cpp; N3485 8.5.4p3/p7.
// A constructor list argument may not narrow through a reference temporary.
// expect exit 0
struct Value { Value(int) {} }; struct A { A(const Value&) {} }; int main() { A object{-1}; return 0; }
