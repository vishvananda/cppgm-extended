// expect reject
// N3485 7.3.3p3; reduced GCC inh-ctor33.C (GPL-3.0-or-later).
struct A { A(int); }; struct B : A {}; struct C : B { using A::A; };
