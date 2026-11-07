// N3485 focus: 9.2p8,8.4.1p1: virtual-function declaration ownership.
// expect exit 0
struct B{virtual int f(int){return 1;}}; template<class T>struct D:B{int f(T)override final{return 0;}}; int main(){D<int>d;B*p=&d;return p->f(0);}
