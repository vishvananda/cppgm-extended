// N3485 13.6p11, 13.3.1.2 and 5.5: built-in ->* conversion candidates. expect exit 0
struct Pad { int pad; }; struct A { int m; int get(int x) const { return m+x; } }; struct D:Pad,A {};
D a; int calls; struct Box { operator D*() { ++calls; return &a; } };
int main() { a.m=8; a.pad=3; Box b; int (A::*p)(int) const=&A::get; return (b->*p)(4)==12 && calls==1 ? 0:1; }
