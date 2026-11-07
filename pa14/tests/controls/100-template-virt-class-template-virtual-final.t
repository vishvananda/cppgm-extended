// N3485 focus: 9.2p8,8.4.1p1: virtual-function declaration ownership.
// expect exit 0
template<class T>struct A{virtual int f()final{return 0;}}; int main(){A<int>a;return a.f();}
