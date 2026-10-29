struct Left { int padding; };
struct Base { int n; int get() const { return n; } };
struct Derived : Left, Base { int extra; };
struct DataBase {
    typedef int Base::*Pointer;
    Pointer value;
    operator Pointer() const { return value; }
};
struct DataDerived {
    typedef const int Derived::*Pointer;
    Pointer value;
    operator Pointer() const { return value; }
};
struct FunctionBase {
    typedef int (Base::*Pointer)() const;
    Pointer value;
    operator Pointer() const { return value; }
};
struct FunctionDerived {
    typedef int (Derived::*Pointer)() const;
    Pointer value;
    operator Pointer() const { return value; }
};
struct Reference {
    typedef int Base::*Pointer;
    Pointer value;
    operator Pointer&() { return value; }
};
struct Override {
    typedef int Base::*Pointer;
    operator Pointer() const { return 0; }
};
bool operator==(Override, int) { return false; }
bool operator!=(int, Override) { return false; }
int main() {
    DataBase b{&Base::n}, bn{0};
    DataDerived d{&Base::n}, dn{0};
    FunctionBase f{&Base::get}, fn{0};
    FunctionDerived g{&Base::get}, gn{0};
    Reference r{&Base::n}, rn{0};
    if (!(b == d) || !(d == b) || b != d || d != b) return 1;
    if (!(bn == dn) || !(dn == bn) || b == dn || !(dn != b)) return 2;
    if (!(f == g) || !(g == f) || f != g || g != f) return 3;
    if (!(fn == gn) || !(gn == fn) || f == gn || !(gn != f)) return 4;
    if (!(r != 0) || !(nullptr != r) || !(rn == 0) || !(nullptr == rn)) return 5;
    if (Override{} == 0 || 0 != Override{}) return 6;
    Derived object; object.n = 42;
    if (object.*d.value != 42 || (object.*g.value)() != 42) return 7;
    return 0;
}
