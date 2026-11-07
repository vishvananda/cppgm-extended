// N3485 7.1.1p1: ordinary declarations agree; CWG1793 instantiation directives are separate. expect exit 0
template<class T>struct S{static int value;};template<class T>int S<T>::value=7;extern template thread_local int S<int>::value;template thread_local int S<int>::value;int main(){return S<int>::value!=7;}
