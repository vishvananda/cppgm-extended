// Reduced from Clang SemaCXX/GH63151.cpp; N3485 8.5.4p3/p7.
// A constructor list argument may not narrow through a reference temporary.
// expect exit 0
struct A { unsigned value; A(const unsigned& x) : value(x) {} }; int main() { unsigned value = 5; A object{value}; return object.value - 5; }
