// expect reject
// Reduced from LLVM clang/test/SemaCXX/union-member-destructor.cpp (Apache-2.0 WITH LLVM-exception); CWG1424/1866.
template<class T> struct D { ~D() { static_assert(sizeof(T) != sizeof(int), "constructor cleanup destructor"); } };
union U { D<int> d; U():d(){} ~U(){} };
