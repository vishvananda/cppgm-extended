// expect reject
// Reduced from LLVM clang/test/SemaCXX/union-member-destructor.cpp (Apache-2.0 WITH LLVM-exception); CWG1424/1866.
struct D { D() {} private: ~D() {} }; union U { D d; U():d(){} ~U(){} };
