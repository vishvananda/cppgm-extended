// expect exit 0
// N3485 14.1p4,p8: explicit ordinary function/pointer/reference parameter types stay valid.
template<int Fn()> struct FunctionParameter {};
template<int (*Fn)()> struct PointerParameter {};
template<int (&Fn)()> struct ReferenceParameter {};
int main() { return 0; }
