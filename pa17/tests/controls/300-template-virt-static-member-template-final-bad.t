// N3485 focus: 9.2p8,8.4.1p1: virtual-function declaration ownership.
// expect reject
struct A{template<class T>static int f(T) final{return 0;}}; int main(){return A::f(0);}
