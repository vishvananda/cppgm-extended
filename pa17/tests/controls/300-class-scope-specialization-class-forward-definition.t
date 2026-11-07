// expect exit 0
// Adopted CWG727: explicit specialization shares the primary template definition scope.
struct Host { template<class T> struct Item { int value(){return 1;} }; template<> struct Item<int>; }; template<> struct Host::Item<int> { int value(){return 6;} }; int main(){Host::Item<int> a;return a.value()!=6;}
