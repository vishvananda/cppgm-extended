// N3485 13.6p11, 13.3.1.2 and 5.5: built-in ->* conversion candidates. expect exit 0
struct A { int m; }; int calls;
struct Member { operator int A::*() { ++calls; return &A::m; } };
int main() { A a; a.m=6; Member m; int x=(&a)->*m; return x==6 && calls==1 ? 0:1; }
