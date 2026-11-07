// N3485 8.3.2p5/p6: direct reference formation versus permitted collapsing. expect reject
template<class T>void f(T & &&value){}
int main(){int x=0;f<int>(x);}
