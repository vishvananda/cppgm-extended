// expect exit 0
// N3485 14.8.1p6,p9: fixed explicit prefix and deduced residual pack.
template<class...T>struct Tuple{};
template<class...T>int f(Tuple<T...>){return sizeof...(T);}
int main(){Tuple<int,float,double> t;Tuple<> e;return f(t)==3&&f<>(t)==3&&f<int>(t)==3&&f<int,float>(t)==3&&f<int,float,double>(t)==3&&f<>(e)==0?0:1;}
