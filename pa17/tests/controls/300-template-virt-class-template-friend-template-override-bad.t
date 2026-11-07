// N3485 focus: 9.2p8,8.4.1p1: virtual-function declaration ownership.
// expect reject
template<class C>struct A{template<class T>friend int f(A,T) override{return 0;}}; int main(){return f(A<int>{},0);}
