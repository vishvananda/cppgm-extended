struct A { int n; }; struct B { int n; };
struct X { typedef int A::*Pointer; operator Pointer() const { return &A::n; } };
struct Y { typedef int B::*Pointer; operator Pointer() const { return &B::n; } };
int main() { return X{} == Y{}; }
