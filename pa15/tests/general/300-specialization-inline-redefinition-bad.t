// AUDIT: a specialization retains its own inline declaration, independently of the primary template.
template<class T> int duplicate(T);
template<> inline int duplicate<int>(int);
template<> int duplicate<int>(int x){return x;}
template<> int duplicate<int>(int x){return x;}
