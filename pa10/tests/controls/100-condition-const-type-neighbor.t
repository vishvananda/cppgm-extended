// expect exit 0
// N3485 declaration grammar/category/access regression; GCC cpp0x gap reduction (GPL-3.0-or-later).
int main() { if (const int value = 3) return value == 3 ? 0 : 1; return 2; }
