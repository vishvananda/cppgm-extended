// N3485 14.7.3p1/p5, CWG1638: explicit specializations permit qualified opaque enum declarations.
// expect exit 0
template<class T>struct A{enum class E;}; template<>enum class A<int>::E; template<>enum class A<int>::E{zero}; int main(){return int(A<int>::E::zero);}
