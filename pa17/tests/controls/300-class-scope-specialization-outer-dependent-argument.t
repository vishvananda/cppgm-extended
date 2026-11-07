// expect exit 0
// Adopted CWG727: specialize where this primary may be defined.
template<class T> struct Host { template<class U> struct Item { int value(){return 1;} }; template<> struct Item<T> { int value(){return sizeof(T)+4;} }; }; int main(){Host<int>::Item<int> a;Host<char>::Item<char> b;Host<char>::Item<int> c;return a.value()!=sizeof(int)+4||b.value()!=5||c.value()!=1;}
