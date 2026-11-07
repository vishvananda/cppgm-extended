// expect reject
// N3485 14.8.1p6,p9: fixed explicit prefix and deduced residual pack.
template<class...T>struct Tuple{};
template<class...T>int f(Tuple<T...>){return 0;}
int main(){Tuple<float,double> t;return f<int>(t);}
