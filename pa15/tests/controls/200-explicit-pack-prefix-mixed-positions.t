// expect exit 0
// N3485 14.8.1p6,p9: fixed explicit prefix and deduced residual pack.
template<class...T>struct Tuple{};
template<class...T>int f(Tuple<T...>,T...){return sizeof...(T);}
int main(){Tuple<int,double> t;return f<int>(t,char(1),2.0)==2?0:1;}
