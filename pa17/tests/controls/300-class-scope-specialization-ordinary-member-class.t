// expect exit 0
// Adopted CWG727: explicit specialization shares the primary template definition scope.
struct Host { template<class T> struct Item { int value(){return 1;} }; template<> struct Item<int> { int value(){return 4;} }; }; int main(){Host::Item<int> a;Host::Item<char> b;return a.value()!=4||b.value()!=1;}
