// N3485 focus: 7.1.2p5,14.5.2p3: virtual-function declaration ownership.
// expect reject
template<class T>virtual int f(T){return 0;} int main(){return f(0);}
