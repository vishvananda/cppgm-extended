// expect exit 0
// I13: N3485 12.8: mutable member uses the nonconst constructor-template source; ordinary const copy does not.
int selected=0;
struct A { A() {} template<class T> A(T&) { ++selected; } };
struct Mutable { mutable A member; }; struct Plain { A member; };
int main() { Mutable first; Mutable second(first); Plain x; Plain y(x); return selected!=1; }
