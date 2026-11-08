// expect exit 0
// N3485 declaration grammar/category/access regression; GCC cpp0x gap reduction (GPL-3.0-or-later).
extern "C" { int f(); } extern "C++" { int g(); } int main() { return 0; }
