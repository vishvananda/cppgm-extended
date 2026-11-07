// Clang GH63151 neighboring nested-list control; N3485 8.5.4p3/p7.
// expect reject
struct A { unsigned value; A(const unsigned& x) : value(x) {} }; int main() { A object{{-1}}; return 0; }
