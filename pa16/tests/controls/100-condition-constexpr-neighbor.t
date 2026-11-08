// expect exit 0
// N3485 declaration grammar/category/access regression; GCC cpp0x gap reduction (GPL-3.0-or-later).
constexpr int three() { return 3; } int main() { if (constexpr int value = three()) return value == 3 ? 0 : 1; return 2; }
