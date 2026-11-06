// CXX11-033; N3485 8.4.3p4. expect reject
// Reduced from LLVM21.1.8 clang/test/CXX/dcl.decl/dcl.fct.def/dcl.fct.def.delete/p4.cpp.
// A deleted specialization's definition must be its first declaration.
template<class T> void action();
template<> void action<int>();
template<> void action<int>() = delete;
