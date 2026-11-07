// N3485 focus: 9.2p8,8.4.1p1: virtual-function declaration ownership.
// expect exit 0
struct A{template<class T>int f(T){return 0;}}; int main(){A a;return a.f(0);}
