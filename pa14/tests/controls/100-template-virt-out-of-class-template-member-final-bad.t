// N3485 focus: 9.2p8,8.4.1p1: virtual-function declaration ownership.
// expect reject
struct B{virtual int f(){return 1;}}; template<class T>struct D:B{int f()override;}; template<class T>int D<T>::f()final{return 0;} int main(){D<int>d;return d.f();}
