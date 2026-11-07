// N3485 13.6p11, 13.3.1.2 and 5.5: built-in ->* conversion candidates. expect exit 0
struct A { int m; }; A a; int calls;
struct Box { operator A*() { ++calls; return &a; } };
int operator->*(Box,int A::*) { return 11; }
int main() { Box b; return (b->*&A::m)==11 && calls==0 ? 0:1; }
