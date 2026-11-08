// expect exit 0
// N3485 14.8.2.1: a previously deduced complete pack remains exact while explicit prefixes extend.
template<class...>struct Tuple{};template<class T,class U>struct Pair{};template<class...T,class...U>int count(Tuple<U...>,Pair<T,U>...){return sizeof...(T)*10+sizeof...(U);}int main(){return count<int>(Tuple<unsigned,unsigned short>(),Pair<int,unsigned>(),Pair<short,unsigned short>())!=22;}
