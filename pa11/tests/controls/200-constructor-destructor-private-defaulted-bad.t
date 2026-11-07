// expect reject
// CWG1424: a declared defaulted destructor retains its access.
struct D { D() {} private: ~D()=default; }; struct S { D d; S():d(){} ~S(){} };
