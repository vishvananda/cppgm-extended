// expect exit 0
// N3485 14.8.1p9; CWG1386 NAD affirms zipped explicit-prefix extension.
template<class T,class U>struct Pair{}; template<class...T,class...U>int count(Pair<T,U>...){return sizeof...(T)*10+sizeof...(U);} int main(){return count<int>(Pair<int,unsigned>(),Pair<short,unsigned short>())!=22;}
