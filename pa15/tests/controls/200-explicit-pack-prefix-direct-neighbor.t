// expect exit 0
// N3485 14.8.1p6,p9: fixed explicit prefix and deduced residual pack.
template<class...T>int f(T...){return sizeof...(T);}
int main(){return f<int*>(0,'a',2.0)==3&&f<int>(char(1),2.0)==2?0:1;}
