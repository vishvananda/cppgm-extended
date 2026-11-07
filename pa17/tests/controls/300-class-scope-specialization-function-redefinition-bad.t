// expect reject
// Adopted CWG727: explicit specialization shares the primary template definition scope.
struct Host { template<class T> int f(T){return 1;} template<> int f<int>(int){return 2;} template<> int f<int>(int){return 3;} }; int main(){}
