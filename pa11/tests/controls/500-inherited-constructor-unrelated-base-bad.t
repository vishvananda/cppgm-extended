// expect reject
// N3485 7.3.3p3: constructor using must name a direct base.
struct A { A(int); }; struct B {}; struct C : B { using A::A; };
