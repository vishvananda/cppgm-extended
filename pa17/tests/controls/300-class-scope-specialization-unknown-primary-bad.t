// expect reject
// Adopted CWG727: explicit specialization shares the primary template definition scope.
struct Host { template<> int f<int>(int){return 2;} }; int main(){}
