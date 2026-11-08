// expect exit 0
// N3485 14.8.2p8: per-position explicit substitution is an immediate context.
template<class T,class U>struct Pair{};template<class...T,class...U>int f(Pair<T*,U>...){return 1;}template<class...T>int f(...){return 7;}int main(){return f<int&>(Pair<int,unsigned>())!=7;}
