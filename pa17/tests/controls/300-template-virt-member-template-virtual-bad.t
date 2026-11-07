// N3485 focus: 14.5.2p3: virtual-function declaration ownership.
// expect reject
struct A{template<class T>virtual int f(T){return 0;}}; int main(){A a;return a.f(0);}
