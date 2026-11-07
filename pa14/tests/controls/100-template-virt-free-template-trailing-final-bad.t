// N3485 focus: 9.2p8,8.4.1p1: virtual-function declaration ownership.
// expect reject
struct S{}; template<class T>auto f(T)->struct S final{return S{};} int main(){f(0);}
