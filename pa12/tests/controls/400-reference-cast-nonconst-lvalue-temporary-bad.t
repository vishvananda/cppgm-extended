// expect reject
// I13: Reference-cast temporary construction does not allow a nonconst lvalue reference to bind a new object.
struct S { S(int) {} }; int main() { S& object=static_cast<S&>(7); }
