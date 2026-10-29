struct S { int n; typedef int S::*Pointer; operator Pointer() const { return &S::n; } };
int main() { return S{} < 0; }
