// expect exit 0; N3485 8.5p14,p17 and8.5.2: character arrays initialized by a single string literal.
// Reduced from LLVM clang/test/CXX/dcl.decl/dcl.init/dcl.init.string/p1.cpp (Apache-2.0 WITH LLVM-exception).
template<class C> int f(){C text[]("ab"); return sizeof(text)==3 && text[0]=='a' && text[2]==0;} int main(){return !f<char>();}
