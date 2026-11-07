// expect exit 0
// N3485 14.8.1p6,p9: fixed explicit prefix and deduced residual pack.
template<class R,class...T>int f(R(*)(T...)){return sizeof...(T);}
int call(int,float,double){return 0;}
int main(){return f(&call)==3&&f<int>(&call)==3&&f<int,int>(&call)==3&&f<int,int,float>(&call)==3&&f<int,int,float,double>(&call)==3?0:1;}
