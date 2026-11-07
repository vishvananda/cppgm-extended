// expect exit 0
// Adopted CWG727: explicit specialization shares the primary template definition scope.
template<class T> struct Host { template<unsigned N> struct Item { int value(){return N+1;} }; template<> struct Item<0> { int value(){return sizeof(T)+7;} }; }; int main(){Host<char>::Item<0> a;Host<char>::Item<1> b;Host<int>::Item<0> c;return a.value()!=8||b.value()!=2||c.value()!=sizeof(int)+7;}
