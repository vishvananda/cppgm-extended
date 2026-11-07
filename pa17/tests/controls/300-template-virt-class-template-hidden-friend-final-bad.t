// N3485 focus: 9.2p8,8.4.1p1: virtual-function declaration ownership.
// expect reject
template<class T>struct A{friend int f(A) final{return 0;}}; int main(){return f(A<int>{});}
