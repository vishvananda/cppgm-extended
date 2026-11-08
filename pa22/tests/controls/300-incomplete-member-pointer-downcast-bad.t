// expect reject
// N3485 5.2.9p12 requires a complete derived class for a member-pointer downcast.
struct Base { int a; }; struct Derived:Base { int b; static_assert((int Base::*)&Derived::b,"derived still incomplete"); }; int main() { return 0; }
