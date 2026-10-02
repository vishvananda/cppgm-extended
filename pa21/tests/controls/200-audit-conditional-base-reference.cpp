// AUDIT-ID: REF-BASE-COND
// AUDIT-EXPECT: run
struct S { int value; S(int n):value(n){} ~S(){} };
struct D:S { D():S(5){} };
int choice(){return 1;}
int main(){const S& s=static_cast<const S&>(choice()?D():throw 99);return s.value!=5;}
