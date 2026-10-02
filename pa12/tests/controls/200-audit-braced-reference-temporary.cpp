// AUDIT-ID: REF-BRACE
// AUDIT-EXPECT: run
struct S { int value; S(int n):value(n){} ~S(){} };
int main(){const S& s{S(5)};return s.value!=5;}
