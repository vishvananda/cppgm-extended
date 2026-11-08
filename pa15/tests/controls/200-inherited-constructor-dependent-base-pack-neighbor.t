// expect exit 0
// Reduced GCC inh-ctor36.C (GPL-3.0-or-later): check the direct base after substitution.
struct A { int n; A(int x) : n(x) {} };
template<class... T> struct C : T... { using A::A; };
int main() { C<A> c(42); return c.n == 42 ? 0 : 1; }
