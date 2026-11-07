// expect reject
// CXX11-085; N3485 11.3p6: friend definitions require a nonlocal class and unqualified name.
void target();struct A{friend void ::target(){}};int main(){}
