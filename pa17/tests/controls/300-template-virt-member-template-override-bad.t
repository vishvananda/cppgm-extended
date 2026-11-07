// N3485 focus: 9.2p8,14.5.2p3: virtual-function declaration ownership.
// expect reject
struct B{virtual int f(int){return 1;}}; struct D:B{template<class T>int f(T) override{return 0;}}; int main(){D d;return d.f(0);}
