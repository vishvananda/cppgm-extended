// N3485 13.6p11, 13.3.1.2 and 5.5: built-in ->* conversion candidates. expect exit 0
struct A { int m; }; A a; int left_calls,right_calls;
struct Box { operator const A*() { ++left_calls; return &a; } };
struct Member { operator volatile int A::*() { ++right_calls; return &A::m; } };
int main() { a.m=7; Box b; Member m; const volatile int &r=b->*m; return r==7 && left_calls==1 && right_calls==1 ? 0:1; }
