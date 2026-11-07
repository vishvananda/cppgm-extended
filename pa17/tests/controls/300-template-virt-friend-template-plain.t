// N3485 focus: 9.2p8,8.4.1p1: virtual-function declaration ownership.
// expect exit 0
struct A{template<class T>friend int f(A,T){return 0;}}; int main(){return f(A{},0);}
