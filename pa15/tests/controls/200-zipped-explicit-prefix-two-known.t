// expect exit 0
// N3485 14.8.1p9: multiple fixed elements lead the deduced tail.
template<class T,class U>struct Pair{}; template<class...T,class...U>int count(Pair<T,U>...){return sizeof...(T)*10+sizeof...(U);} int main(){return count<int,char>(Pair<int,unsigned>(),Pair<char,short>(),Pair<long,bool>())!=33;}
