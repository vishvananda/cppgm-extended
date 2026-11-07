// N3485 13.6p11, 13.3.1.2 and 5.5: built-in ->* conversion candidates. expect reject
struct A { int m; }; struct Box { operator const A*(); };
void f(Box b,int A::*p) { (b->*p)=1; }
