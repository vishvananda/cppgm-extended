// expect exit 0
// CWG727: copied specialization definitions retain per-enclosing-instance facts.
template<class T> struct Host { template<unsigned N> struct Item { int value(){return 1;} }; template<> struct Item<0> { int value(){return sizeof(T)+7;} }; }; int main(){int a=Host<int>::Item<0>().value();int b=Host<char>::Item<0>().value();int c=Host<int>::Item<0>().value();int d=Host<char>::Item<0>().value();return a!=sizeof(int)+7||b!=8||c!=a||d!=b;}
