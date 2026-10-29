struct A { int n; }; struct B { int n; };
struct X {
    typedef int A::*First; typedef int B::*Second;
    operator First() const { return &A::n; }
    operator Second() const { return &B::n; }
};
int main() { return X{} == 0; }
