// N3485 8.5p14,p17 and8.5.2: character arrays initialized by a single string literal.
// Reduced from LLVM clang/test/CXX/dcl.decl/dcl.init/dcl.init.string/p1.cpp (Apache-2.0 WITH LLVM-exception).
wchar_t a[](L"ab"); char16_t b[](u"ab"); char32_t c[](U"ab"); extern wchar_t a[3]; extern char16_t b[3]; extern char32_t c[3]; int main(){return 0;}
