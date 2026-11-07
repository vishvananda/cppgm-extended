// expect exit 0; N3485 8.5p14,p17 and8.5.2: character arrays initialized by a single string literal.
// Reduced from LLVM clang/test/CXX/dcl.decl/dcl.init/dcl.init.string/p1.cpp (Apache-2.0 WITH LLVM-exception).
char16_t text[](u"\U00010000"); char32_t point[](U"\U00010000"); wchar_t wide[](L"xy"); int main(){return sizeof(text)/sizeof(text[0])!=3 || text[0]!=0xd800 || text[1]!=0xdc00 || text[2]!=0 || point[0]!=0x10000 || point[1]!=0 || wide[1]!=L'y' || wide[2]!=0;}
