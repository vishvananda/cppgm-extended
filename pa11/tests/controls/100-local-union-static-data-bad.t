// expect reject
// N3485 9.8p4 applies to a local union's static data member too.
void f() { union Local { int member; static int value; }; }
