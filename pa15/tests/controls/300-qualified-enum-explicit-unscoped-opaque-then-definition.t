// N3485 14.7.3p1/p5, CWG1638: explicit specializations permit qualified opaque enum declarations.
// expect exit 0
template<class T>struct A{enum E:int;}; template<>enum A<int>::E:int; template<>enum A<int>::E:int{zero}; int main(){return A<int>::zero;}
