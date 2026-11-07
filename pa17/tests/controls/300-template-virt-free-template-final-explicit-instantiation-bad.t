// N3485 focus: 9.2p8,8.4.1p1: virtual-function declaration ownership.
// expect reject
template<class T>int f(T) final{return 0;} template int f<int>(int); int main(){return f(0);}
