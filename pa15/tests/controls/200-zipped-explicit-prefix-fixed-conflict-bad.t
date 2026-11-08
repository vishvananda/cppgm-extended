// expect reject
// N3485 14.8.2.5: a fixed nested template argument cannot change to match the argument.
template<class T,class U>struct Pair{}; template<class...T,class...U>void f(Pair<T,U>...){} int main(){f<int>(Pair<char,unsigned>());}
