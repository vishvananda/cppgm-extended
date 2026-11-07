// expect reject
// Clang class.local/p4.cpp; N3485 9.8p4: a local class has no static data members.
void f() { struct Local { static int value; }; }
