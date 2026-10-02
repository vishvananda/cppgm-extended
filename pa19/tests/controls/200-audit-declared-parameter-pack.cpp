// AUDIT-ID: MANGLE-PACK
// AUDIT-EXPECT: symbols
template<class T,class U=int>struct Box{};
template<class T,class... U>void take(const Box<T,U...>&){}
void (*take_address)(const Box<int,int>&)=&take<int,int>;
void force(){Box<int> value;take(value);}
