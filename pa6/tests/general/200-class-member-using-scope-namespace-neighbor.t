// N3485 7.3.3p8 and adopted CWG1742: class-member using declarations require class scope, except enumerators. expect exit 0
namespace N{typedef int T;void f();}using N::T;using N::f;
