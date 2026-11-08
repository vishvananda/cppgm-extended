// expect exit 0
// I13: N3485 5.2.9/8.5.3: reference static_cast can construct a class temporary and extend its lifetime.
int live=0, dead=0; struct S { int n; S(int v):n(v) { ++live; } ~S() { --live; ++dead; } };
int main() { { const S& a=static_cast<const S&>(7); S&& b=static_cast<S&&>(9); if(live!=2 || dead!=0 || a.n!=7 || b.n!=9) return 1; } return live!=0 || dead!=2; }
