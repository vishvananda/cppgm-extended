// expect exit 0
// Reduced from LLVM clang/test/SemaCXX/union-member-destructor.cpp (Apache-2.0 WITH LLVM-exception); CWG1424/1866.
template<class T> struct D { ~D() { static_assert(sizeof(T) != sizeof(int), "constructor cleanup destructor"); } };
struct S { D<int>& d; D<int>* p; S(D<int>& x):d(x),p(&x){} ~S(){} }; int main(){return 0;}
