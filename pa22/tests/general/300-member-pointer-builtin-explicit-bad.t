struct S { int n; typedef int S::*Pointer; explicit operator Pointer() const { return &S::n; } };
int main() { return S{} == 0; }
