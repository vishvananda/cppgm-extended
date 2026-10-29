// AUDIT: a specialization retains its own inline declaration, independently of the primary template.
struct Member{template<class T> int pick(T)const;};
template<> inline int Member::pick(int)const;
template<> int Member::pick(int value)const{return value;}
template<> int Member::pick(int)const;
int main(){Member m;return m.pick(42)!=42;}
