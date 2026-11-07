// expect reject
// Adopted CWG727: explicit specialization shares the primary template definition scope.
struct Host { template<class T> struct Item {}; }; struct Other { template<> struct Host::Item<int> {}; }; int main(){}
