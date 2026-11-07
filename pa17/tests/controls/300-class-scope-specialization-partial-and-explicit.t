// expect exit 0
// Adopted CWG727: explicit specialization shares the primary template definition scope.
struct Host { template<class T> struct Item { int value(){return 1;} }; template<class T> struct Item<T*> { int value(){return 2;} }; template<> struct Item<int*> { int value(){return 9;} }; }; int main(){Host::Item<char*> a;Host::Item<int*> b;Host::Item<int> c;return a.value()!=2||b.value()!=9||c.value()!=1;}
