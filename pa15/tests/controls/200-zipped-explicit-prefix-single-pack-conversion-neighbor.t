// expect exit 0
// N3485 14.8.1p6/p9: explicit scalar prefix arguments convert, the residual is deduced.
template<class...T>int count(T...){return sizeof...(T);} int main(){return count<int,int>(short(1),char(2),3.0)!=3;}
