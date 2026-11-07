// expect reject
// Adopted CWG727: specialize where this primary may be defined.
template<class T> int action(T){return 1;} struct Host { template<> int action<int>(int){return 2;} }; int main(){}
