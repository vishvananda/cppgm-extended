// expect exit 0
// CWG1424: a declared defaulted destructor retains its access.
struct D { D() {} ~D()=default; }; struct S { D d; S():d(){} ~S(){} }; int main(){S s;}
