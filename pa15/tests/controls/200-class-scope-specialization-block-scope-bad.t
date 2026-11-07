// expect reject
// Adopted CWG727: explicit specialization shares the primary template definition scope.
template<class T> int f(T){return 1;} int main(){template<> int f<int>(int);return 0;}
