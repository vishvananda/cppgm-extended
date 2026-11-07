// expect exit 0
// Adopted CWG727: explicit specialization shares the primary template definition scope.
template<class T> int f(T){return 1;} template<> int f<int>(int){return 2;} int main(){return f<int>(0)!=2||f<char>(0)!=1;}
