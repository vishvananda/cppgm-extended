// AUDIT-ID: LOCAL-ODR
// AUDIT-EXPECT: reject
void f() { int x=1; struct A { int get() { return [&] { return x; }(); } }; }
