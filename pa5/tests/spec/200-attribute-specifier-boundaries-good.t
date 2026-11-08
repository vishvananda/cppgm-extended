// expect exit 0
// N3485 declaration grammar/category/access regression; GCC cpp0x gap reduction (GPL-3.0-or-later).
[[vendor::tag]] int n; struct [[vendor::tag]] S { [[vendor::tag]] int value; int [[vendor::tag]] other; }; int main() { return 0; }
