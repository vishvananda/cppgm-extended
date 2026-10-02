// AUDIT-ID: CONST-MEMBER-BOOL
// AUDIT-EXPECT: compile
struct S { int x; }; static_assert(&S::x, "nonnull member pointer"); int main(){ return 0; }
