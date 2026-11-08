// expect reject
// N3485 7.3.3p3: a class is not its own direct base.
struct A { using A::A; };
