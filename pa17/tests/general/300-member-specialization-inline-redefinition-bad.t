// AUDIT: a specialization retains its own inline declaration, independently of the primary template.
struct Member{template<class T>int duplicate(T);};
template<> inline int Member::duplicate<int>(int);
template<> int Member::duplicate<int>(int x){return x;}
template<> int Member::duplicate<int>(int x){return x;}
