// expect reject
// N3485 9.8p4 also forbids an initialized integral constant member.
void f() { struct Local { static const int value = 3; }; }
