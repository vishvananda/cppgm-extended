// expect exit 0
// Adopted CWG727: explicit specialization shares the primary template definition scope.
struct Host { template<class T> int f(T){return 1;} template<> int f<int>(int); }; template<> int Host::f<int>(int){return 8;} int main(){Host h;return h.f<int>(0)!=8||h.f<char>(0)!=1;}
