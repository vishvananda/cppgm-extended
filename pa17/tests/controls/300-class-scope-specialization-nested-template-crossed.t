// expect exit 0
// CWG727: nested member-template instantiation retains both enclosing and own arguments.
template<class T> struct Outer { template<int N> struct Item {}; template<> struct Item<0> { template<class U> struct Nested { int read(){return sizeof(T)*100+sizeof(U);} }; }; };
int main(){Outer<int>::Item<0>::Nested<char> a;Outer<char>::Item<0>::Nested<int> b;Outer<int>::Item<0>::Nested<int> c;Outer<char>::Item<0>::Nested<char> d;return a.read()!=sizeof(int)*100+1||b.read()!=100+sizeof(int)||c.read()!=sizeof(int)*100+sizeof(int)||d.read()!=101||a.read()!=sizeof(int)*100+1;}
