// expect reject
// N3485 14.5.3: explicit prefix cannot exceed the completed zipped pack.
template<class T,class U>struct Pair{}; template<class...T,class...U>void f(Pair<T,U>...){} int main(){f<int,char>(Pair<int,unsigned>());}
