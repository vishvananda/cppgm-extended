// expect exit 0
// N3485 14.8.1p6,p9: fixed explicit prefix and deduced residual pack.
template<int...I>struct Values{};
template<int...I>int f(Values<I...>){return sizeof...(I);}
int main(){Values<1,2,3> v;return f<1>(v)==3&&f<1,2,3>(v)==3?0:1;}
