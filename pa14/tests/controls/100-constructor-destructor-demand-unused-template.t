// expect exit 0
// Reduced from LLVM clang/test/SemaCXX/union-member-destructor.cpp (Apache-2.0 WITH LLVM-exception); CWG1424/1866.
template<class T> struct D { ~D() { typename T::missing value; (void)value; } };
template<class T> union U { D<T> d; U():d(){} ~U(){} }; int main(){return 0;}
