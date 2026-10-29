// AUDIT: a specialization retains its own inline declaration, independently of the primary template.
template<class T> inline int primary(T value){return value;}
template<> int primary<int>(int value){return value;}
template<class T> int retained(T);
template<> inline int retained<int>(int);
template<> int retained<int>(int value){return value;}
template<> int retained<int>(int);
template<class T> struct Box{int get()const;};
template<> inline int Box<int>::get()const;
template<> int Box<int>::get()const{return 42;}
int main(){Box<int>b;return primary(42)!=42||retained(42)!=42||b.get()!=42;}
