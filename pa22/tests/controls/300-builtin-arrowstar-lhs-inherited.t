// N3485 13.6p11, 13.3.1.2 and 5.5: built-in ->* conversion candidates. expect exit 0
struct A { int m; }; A a; int calls;
struct Box { operator A*() { ++calls; return &a; } }; struct Child:Box {};
int main() { a.m=4; Child b; int A::*p=&A::m; (b->*p)=9; return a.m==9 && calls==1 && (&a->*p)==9 ? 0:1; }
