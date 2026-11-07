// N3485 focus: 9.2p8,8.4.1p1: virtual-function declaration ownership.
// expect exit 0
struct B{virtual int f(){return 1;}}; template<class T>struct D:T{int f()override final{return 0;}}; int main(){D<B>d;B*p=&d;return p->f();}
