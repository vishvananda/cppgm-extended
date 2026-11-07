// N3485 8.3.2p5/p6: direct reference formation versus permitted collapsing. expect exit 0
template<class T>T&& forward(T& value){return static_cast<T&&>(value);}
int main(){int x=4;int &a=forward<int&>(x);int &&b=forward<int>(x);a=5;b=6;return &a==&x && &b==&x && x==6?0:1;}
