// expect reject
// Reduced from LLVM clang/test/SemaCXX/union-member-destructor.cpp (Apache-2.0 WITH LLVM-exception); CWG1424/1866.
template<class T> struct D { ~D() { static_assert(sizeof(T) != sizeof(int), "constructor cleanup destructor"); } };
struct S { union { D<int> d; int n; }; S():d(){} ~S(){} };
