// expect reject
// N3485 7.3.3p3: a substituted base pack still must contain the named direct base.
struct A { A(int); }; struct B {};
template<class... T> struct C : T... { using A::A; };
int main() { return sizeof(C<B>); }
