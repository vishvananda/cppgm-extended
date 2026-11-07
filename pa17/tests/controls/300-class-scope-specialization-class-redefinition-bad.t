// expect reject
// Adopted CWG727: explicit specialization shares the primary template definition scope.
struct Host { template<class T> struct Item {}; template<> struct Item<int> {}; template<> struct Item<int> {}; }; int main(){}
