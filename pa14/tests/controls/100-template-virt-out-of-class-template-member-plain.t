// N3485 focus: 9.2p8,8.4.1p1: virtual-function declaration ownership.
// expect exit 0
struct B{virtual int f(){return 1;}}; template<class T>struct D:B{int f()override final;}; template<class T>int D<T>::f(){return 0;} int main(){D<int>d;B*p=&d;return p->f();}
