// N3485 8.5p14,p17 and8.5.2: character arrays initialized by a single string literal.
// Reduced from LLVM clang/test/CXX/dcl.decl/dcl.init/dcl.init.string/p1.cpp (Apache-2.0 WITH LLVM-exception).
char a[]("ab"); signed char b[]("ab"); unsigned char c[]("ab"); extern char a[3]; extern signed char b[3]; extern unsigned char c[3]; int main(){return 0;}
