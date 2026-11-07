// expect exit 0
// Reduced from LLVM clang/test/SemaCXX/union-member-destructor.cpp (Apache-2.0 WITH LLVM-exception); CWG1424/1866.
int n; template<class T> struct D { D(){++n;} ~D(){--n;} }; union U { D<char> d; U():d(){} ~U(){d.~D();} }; int main(){ { U u; if(n!=1)return 1; } return n; }
