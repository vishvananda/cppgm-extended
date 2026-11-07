// expect exit 0
// CWG727: copied specialization definitions retain per-enclosing-instance facts.
template<class T> struct Host { template<class U> int action(U){return 1;} template<> int action<int>(int){return sizeof(T)+3;} }; int main(){Host<int> a;Host<char> b;Host<int> c;return a.action<int>(0)!=sizeof(int)+3||b.action<int>(0)!=4||c.action<int>(0)!=sizeof(int)+3||b.action<char>(0)!=1;}
