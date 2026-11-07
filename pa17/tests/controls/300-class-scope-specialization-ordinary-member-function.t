// expect exit 0
// Adopted CWG727: explicit specialization shares the primary template definition scope.
struct Host { template<class T> int f(T){return 1;} template<> int f<int>(int){return 3;} template<> int f(float){return 5;} }; int main(){Host h;return h.f<int>(0)!=3||h.f<float>(0)!=5||h.f<char>(0)!=1;}
