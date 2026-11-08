// expect exit 0
// N3485 7.3.3p3: an alias still denotes the direct base.
struct A { int n; A(int x) : n(x) {} }; typedef A Alias;
struct B : A { using Alias::Alias; };
int main() { B b(6); return b.n == 6 ? 0 : 1; }
