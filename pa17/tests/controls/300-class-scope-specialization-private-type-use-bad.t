// expect reject
// Adopted CWG727: explicit specialization shares the primary template definition scope.
class Host { template<class T> struct Item {}; template<> struct Item<int> {}; }; Host::Item<int> value; int main(){}
