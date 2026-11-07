// N3485 8.3.5p9 and 5.2.2p4: declarations and addresses do not require
// parameter layout; calls and definitions do.
// expect exit 0
struct Arg; struct C { void target(Arg); }; void (C::*global)(Arg)=&C::target; void capture() { void (C::*local)(Arg)=&C::target; global=local; } struct Arg { int number; double real; }; int result; void C::target(Arg a) { result=a.number+int(a.real); } int main() { C c; Arg a={3,4.0}; capture(); (c.*global)(a); return result==7 ? 0 : 1; }
