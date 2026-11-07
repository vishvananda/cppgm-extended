// N3485 focus: 9.2p8,8.4.1p1: virtual-function declaration ownership.
// expect exit 0
template<class C>struct A{template<class T>int g(T){return 0;} virtual int f()final{return 0;}}; int main(){A<int>a;return a.f()+a.g(0);}
