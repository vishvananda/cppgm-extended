// N3485 7.3.3p8 and adopted CWG1742: class-member using declarations require class scope, except enumerators. expect reject
struct S{static int n;};using S::n;
