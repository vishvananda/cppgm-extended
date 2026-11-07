// N3485 focus: 9.2p8,14.5.2p3: virtual-function declaration ownership.
// expect reject
template<class C>struct A{template<class T>int f(T) final{return 0;}}; int main(){A<int>a;return a.f(0);}
