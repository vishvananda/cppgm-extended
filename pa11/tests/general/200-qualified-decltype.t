namespace N { int x; int f(); }
decltype(N::x) y;
decltype((N::x)) z;
decltype(N::f) *fp;
