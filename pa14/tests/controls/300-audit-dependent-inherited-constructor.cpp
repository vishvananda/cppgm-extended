// AUDIT-ID: INHERITED-DEPENDENT
// AUDIT-EXPECT: run
struct B { int n; B(int x):n(x) {} };
template<class T> struct D:T { using T::T; };
int main(){D<B> x(7);return x.n!=7;}
