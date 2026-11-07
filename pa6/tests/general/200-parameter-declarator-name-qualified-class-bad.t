// N3485 8.3p1,3.3.4: a parameter declarator names an ordinary unqualified identifier.
struct A{static int n;}; void f(int A::n); int main(){return 0;}
