// expect reject
// N3485 9.8p3,p4: a class nested in a local class is itself local.
void f() { struct Local { struct Nested { static int value; }; }; }
